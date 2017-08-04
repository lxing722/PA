#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
		wp_pool[i].sum = 0;
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}
/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
	if(free_ == NULL)
		assert(0);
	WP *temp = free_;
	free_ = free_->next;
	temp->next = NULL;
	printf("good\n");
	return temp;
}

void free_wp(int N){
	WP *wp = head;
	while(wp != NULL && wp->NO != N)
		wp = wp->next;
	if(wp == NULL){
		printf("Wrong watchpoint\n");
		return ;
	}
		
	if(wp == head)
		head = wp->next;
	else{
		WP *temp = head;
		while(temp->next != wp)
		temp = temp->next;
		temp->next = wp->next;
	}
	wp->next = free_;
	free_ = wp;
}

void set_wp(char *args){
	WP *p = new_wp();
	printf("good\n");
	strcpy(p->exprs, args);
	printf("good\n");
	bool success = true;
	p->sum = expr(args,&success);
	if(success == false)
		return;
	if(head == NULL){
		head = p;
	}	
	else{
		WP *temp = head;
		while(temp->next != NULL){
			temp = temp->next;
		}
		temp->next = p;
	}
}
void print_wp(){
	if(head != NULL){
		WP *temp = head;
		while(temp != NULL){
			printf("%d\t%s\n",temp->NO,temp->exprs);
			temp = temp->next;
		}
	}
}





