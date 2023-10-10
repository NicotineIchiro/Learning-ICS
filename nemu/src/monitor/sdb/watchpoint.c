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
#include <cpu/cpu.h>
#include <stdbool.h>


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
WP* new_wp(char * s) {
	if (free_ == NULL){
		printf("There's no free watch point.\n");
		assert(0);
	}

	//necessarity of tail?
	//because there's only head now, so...
	//TODO: Don't insert when bad expr.
	bool eval_flag = true;
	word_t expr_result = expr(s, &eval_flag);
	assert(eval_flag == true);
	if (head == NULL) {
		head = wp_pool;
		free_ = wp_pool->next;
		head->next = NULL;
		strncpy(head->expr_str, s, strlen(s));
		head->current_value = expr_result;
		return head;
	}

	WP* temp;

	temp = free_;
	free_ = free_->next;
	temp->next = head;
	head = temp;

	strncpy(head->expr_str, s, strlen(s));
	head->current_value = expr_result;

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
			memset(free_, 0, WP_EXPR_LEN);
			return true;
		}
		prior = p;
	}
	printf("The passed WP is not in wp_pool now!\n");
	return false;
}
void wp_display() {
	if (head == NULL) {
		printf("No maintaining watchpoint now.\n");
		return;
	}

	for (WP* pwp = head; pwp != NULL; pwp = pwp->next) {
		printf("NO:%d\tvalue:%lu\texpr:%s\n", pwp->NO, pwp->current_value, pwp->expr_str);
	}

	return;
}

bool difftest_wp() {
	bool diff_flag = false;
	bool eval_flag = true;
	for (WP* pwp = head; pwp != NULL; pwp = pwp->next) {
		word_t new_value;
		new_value = expr(pwp->expr_str, &eval_flag);
		assert(eval_flag == true);
		
		if (new_value != pwp->current_value) {
			diff_flag = true;

			printf("watchpoint %d: %s\n\n"
					   "Old value = %lu\n"
						 "New value = %lu\n", \
						 pwp->NO, pwp->expr_str, \
						 pwp->current_value, \
						 new_value);
			pwp->current_value = new_value;
		}
	}	
	return diff_flag;
}
