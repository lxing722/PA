#include "nemu.h"
#include<stdlib.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#define MAX_SIZE 32
enum {
	NOTYPE = 256, OPENBAR, NUM, EQ, PLUS, MINUS, POWER, DIVIDE, CLOSEBAR

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
//////////////////////////////mycode
	{" +",	NOTYPE},                // spaces
	{"\\(", OPENBAR},				// openbar
	{"^[0-9]+", NUM},				// number
	{"\\+", PLUS},					// plus
	{"==", EQ},						// equal
	{"\\-", MINUS},					// minus
	{"\\*", POWER},					// power
	{"\\/", DIVIDE},				// divide
	{"\\)", CLOSEBAR}, 				// closebar
////////////////////////////////////////
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[MAX_SIZE];
} Token;

Token tokens[MAX_SIZE];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE:
						break;
					case NUM:
						tokens[nr_token].type = rules[i].token_type;
						if(substr_len > MAX_SIZE)
							assert(0);
						int j;
						for(j = 0; j < substr_len; j++)
							tokens[nr_token].str[j] = substr_start[j];
						nr_token++;
						break;
					case OPENBAR:
					case PLUS:
					case EQ:
					case MINUS:
					case POWER:
					case DIVIDE:
					case CLOSEBAR:
						tokens[nr_token++].type = rules[i].token_type;
						break;
					default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}
static int stack[MAX_SIZE];
static int stack_len = 0;
static void init_stack(){
	stack_len = 0;
}
static void push(int c){
	if(stack_len == MAX_SIZE){
		printf("Stack is full\n");
		return;
	}
	stack[stack_len++] = c;
}
static int pop(){
	if(stack_len == 0){
		printf("Stack is empty\n");
		return -1;
	}	
	return stack[stack_len--];
}
static bool check_parentheses(int start, int end){
	init_stack();
	int i;
	for(i = start; i <= end; i++){
		if(tokens[i].type == OPENBAR)
			push(tokens[i].type);
		if(tokens[i].type == CLOSEBAR){
			if(stack_len == 1&&i != end)
				return false;
			pop();
		}
	}
	return true;
}
static bool check_bar(){
	init_stack();
	int i;
	for(i = 0; i < nr_token; i++){
		if(tokens[i].type == OPENBAR)
			push(tokens[i].type);
		else if(tokens[i].type == CLOSEBAR){
			if(stack_len == 0)
				return false;
			pop();
		}
	}
	if(stack_len != 0)
		return false;
	return true;
}
static bool is_operator(int type){
	if(type == PLUS || type == DIVIDE || type == POWER || type  == MINUS)
		 return true;
	return false;
}
static int domi_op(int start, int end){
	int pos = 0;
	int domi = 10;
	int i;
	for(i = start; i <= end; i++){
		if(tokens[i].type == OPENBAR){
			while(tokens[i].type != CLOSEBAR)
				i++;
		}
		if(is_operator(tokens[i].type) && tokens[i].type <= domi){
			pos = i;
			domi = tokens[i].type;
		}
	}
	return pos;
}

static int eval(int start, int end){
	if(start > end){
		assert(0);
	}
	else if(start == end){
		printf("%d\n",tokens[start].type);
		printf("%s\n",tokens[start].str);
		printf("%d\n",atoi(tokens[start].str));
		return atoi(tokens[start].str);
	}
	else if(check_parentheses(start, end)){
		return eval(start+1, end-1);
	}
	else{
		int op = domi_op(start, end);
		int val1 = eval(start, op-1);
		int val2 = eval(op+1, end);
		switch(tokens[op].type){
			case PLUS:return val1 + val2;
			case MINUS:return val1 - val2;
			case POWER: return val1 * val2;
			case DIVIDE:
				assert(val2);
				return val1 / val2;
			default:assert(0);
		}
	}
}
/////////////////////////////////////////
int expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	if(!check_bar(e)){
		*success = false;
		return 0;
	}

	//panic("please implement me");
	return eval(0,nr_token-1);
}

