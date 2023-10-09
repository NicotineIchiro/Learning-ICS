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

#include "sdb.h"

#define NR_WP 32
#define EXPR_LEN 512
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
	//about the variable pointed.
	char expr_str[EXPR_LEN];
	//is value necessary?
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool; //from array head.
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(const char * s) {
	if (free_ == NULL){
		printf("There's no free watch point.\n");
		assert(0);
	}

	//necessarity of tail?
	//because there's only head now, so...
	if (head == NULL) {
		head = wp_pool;
		free_ = wp_pool->next;
		head->next = NULL;
		strncpy(head->expr_str, s, strlen(s));		
		return head;
	}

	WP* temp;

	temp = free_;
	free_ = free_->next;
	temp->next = head;
	head = temp;

	strncpy(head->expr_str, s, strlen(s));

	return head;
}
bool free_wp(WP *wp) {
	if (wp == NULL) {
		printf("NULL watchpoint.\n");
		return false;
	}
	if (head == NULL) {
		printf("There's no used watchpoint.\n");
		return false;
	}

	for (WP* p = head, *prior = NULL; p != NULL; p = p->next) {
		if (p == wp) {
			if (p == head) {
				head = head->next;
				p->next = free_;
				free_ = p;
			}
			else {
				prior->next = p->next;
				p->next = free_;
				free_ = p;
			}
			memset(free_, 0, EXPR_LEN);
			return true;
		}
		prior = p;
	}
	printf("The passed WP is not in wp_pool now!\n");
	return false;
}
