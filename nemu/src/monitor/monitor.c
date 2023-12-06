/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
	Log("Watchpoint: %s", MUXDEF(CONFIG_WATCHPOINT, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
 // Log("Exercise: Please remove me in the source code and compile NEMU again.");
 // assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();
static char *elf_file = NULL;
static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;

static long load_img() {

  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}
#include <elf.h>
int symtab_len = 0;
char * Strtab = NULL;
Elf64_Sym * Symtab = NULL;
Elf64_Ehdr * elf_fhp = NULL;
void free_symstrtabs(){
	free(Strtab);
	free(Symtab);
	free(elf_fhp);
}
static long load_elf() {
	if (elf_file == NULL) {
		Assert(0, "Unable to load '%s'", elf_file);
	}

	FILE *fp = fopen(elf_file, "rb");
	Assert(fp, "Can not open '%s'", elf_file);
	//fseek: set the FILE INDICATOR in pos [whence + off] in fo
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);//get off set of [FILE INICATOR to STREAM]
	Log("The elf file is %s, size = %ld", elf_file, size);

	//read ELF header
	fseek(fp, 0, SEEK_SET);
	elf_fhp = (Elf64_Ehdr *)malloc(sizeof(Elf64_Ehdr));
	int sign = fread(elf_fhp, sizeof(Elf64_Ehdr), 1, fp);
	Assert(sign == 1, "Error when reading ELF header!");
	Assert(elf_fhp->e_type == ET_EXEC, "The file type is not executable!");
//	printf("e_entry: %p\n"
//				 "e_phoff: 0x%lx\n"
//				 "e_shoff: 0x%lx\n"
//				 "e_ehsize: %hu\n"
//				 "e_phentsize: %hu\n"
//				 "e_shentsize: %hu\n", (void *)elf_fhp->e_entry, elf_fhp->e_phoff, elf_fhp->e_shoff, elf_fhp->e_ehsize, elf_fhp->e_phentsize, elf_fhp->e_shentsize);

	//Section header table is AN ARRAY OF ElfN_Shdr begin at e_shoff
	Elf64_Shdr * elf_shtp = NULL;
	Elf64_Phdr * elf_phtp = NULL;

	//get the two header table if none void.
	//TODO: something wrong when reading shtp.
	//fread arg2 is single size, arg3 is number of item to read.
	if (elf_fhp->e_shnum != 0) {
		elf_shtp = (Elf64_Shdr *)malloc(elf_fhp->e_shentsize * elf_fhp->e_shnum);
		fseek(fp, elf_fhp->e_shoff, SEEK_SET);
		int ret = fread(elf_shtp, elf_fhp->e_shentsize, elf_fhp->e_shnum, fp);
		Assert(ret == elf_fhp->e_shnum, "Error when reading section headers!");
	}
	if (elf_fhp->e_phnum != 0){
		elf_phtp = (Elf64_Phdr *)malloc(elf_fhp->e_phentsize * elf_fhp->e_phnum);
		fseek(fp, elf_fhp->e_phoff, SEEK_SET);
		int ret = fread(elf_phtp, elf_fhp->e_phentsize, elf_fhp->e_phnum, fp);
		Assert(ret == elf_fhp->e_phnum, "Error when reading program headers!");
	}

	Elf64_Shdr textSh;
	Elf64_Off textOffset = 0;//TODO: universal algo to find .text
	//printf("sht[1].offset: 0x%lx\n", elf_shtp[1].sh_offset); //magic num. How can I know sht[1] is .text ?	
	//
	//Read the section header string table first..
	Elf64_Shdr shstr = elf_shtp[elf_fhp->e_shstrndx];
	Elf64_Addr shstrtab_base = shstr.sh_offset;
	char * shstrtab = (char *)malloc(shstr.sh_size + 1);// read the whole tab.
	memset(shstrtab, '\0', shstr.sh_size + 1);
	fseek(fp, shstrtab_base, SEEK_SET);
	int hret = fread(shstrtab, shstr.sh_size, 1, fp);
	Assert(hret == 1, "Error when reading shstrtab!");

	int text_found = 0, symtab_found = 0, strtab_found = 0;
	for (int i = 0, shnum = elf_fhp->e_shnum; i < shnum; ++i) {//to find the .text header	
		//unable to directly access, so should I fread? -> Yes!	
		char * shstr_begin;
		shstr_begin = shstrtab + elf_shtp[i].sh_name;
		//confirm the section header is of .text
		if (!text_found && strcmp((const char *)shstr_begin, ".text") == 0) {
			text_found = 1;
			textSh = elf_shtp[i];
			textOffset = textSh.sh_offset;	
		}
		
		//read the symtable
		//WARNING: symtab is not string! is ARRAY OF ElfN_Sym!
		if (!symtab_found && strcmp((const char *)shstr_begin, ".symtab") == 0) {
			symtab_found = 1;
			Elf64_Shdr symhdr = elf_shtp[i];
			Symtab = (Elf64_Sym *)malloc(symhdr.sh_size);
			symtab_len = symhdr.sh_size;
			memset(Symtab, '\0', symhdr.sh_size);
			fseek(fp, symhdr.sh_offset, SEEK_SET);
			int ret = fread(Symtab, sizeof(Elf64_Sym), symhdr.sh_size / sizeof(Elf64_Sym), fp);
			Assert(ret == symhdr.sh_size / sizeof(Elf64_Sym), "Error when reading symtab!");
		}
		//read the string table.
		if (!strtab_found && strcmp((const char *)shstr_begin, ".strtab") == 0) {
			strtab_found = 1;
			Elf64_Shdr strhdr = elf_shtp[i];
			Strtab = (char *)malloc(strhdr.sh_size + 1);
			memset(Strtab, '\0', strhdr.sh_size + 1);
			fseek(fp, strhdr.sh_offset, SEEK_SET);
			int ret = fread(Strtab, strhdr.sh_size, 1, fp);
			Assert(ret == 1, "Error when reading strtab!");
		}
		if (text_found && symtab_found && strtab_found) break;
	}

	//set the file indicator in the begin of code(.text)
	fseek(fp, textOffset, SEEK_SET);//0x1000 is begin of the .text of the elf now.
		 																							//the begin to .text don't need to read in pmem.
	//load the text into NEMU pmem.
	int ret = fread(guest_to_host(RESET_VECTOR), size - textOffset, 1, fp);
	assert(ret == 1);
#ifdef CONFIG_FTRACE
	extern char ident_str[257];
	for (int i = 0; i < 257; ++i) {
		ident_str[i] = i == 256 ? '\0' : ' ';
	}
#endif
	//the global symtab and strtab free in engine_start()
	free(shstrtab);
	free(elf_shtp);
	free(elf_phtp);
	//free(elf_fhp);
	fclose(fp);
	return size;	
}
static int parse_args(int argc, char *argv[]) {
				
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
		// by me
		{"elf"			, required_argument, NULL, 'e'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:e", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
			//by me
			case 'e': elf_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
	
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);
	
  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
	long img_size;
	if (elf_file == NULL)
		img_size = load_img();
	else
		img_size = load_elf();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
