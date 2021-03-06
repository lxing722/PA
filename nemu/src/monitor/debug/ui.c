#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
 
void cpu_exec(uint32_t);
void init_wp_pool();
void free_wp(int N);
void set_wp(char *args);
void print_wp();
int check_wp();
/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

////////////////////////////////////my code
/*Function for command si*/
static int cmd_si(char *args){
	char *arg = strtok(NULL, " ");
	int steps = 1;
	if(arg != NULL){
		steps = atoi(arg);
		if(steps < 0){
			printf("N must larger than or equal to 0\n");
			return 0;
		}
	}
	cpu_exec(steps);
	return 0;
}
/*Print all the registers' value*/
static void info_all_r(){
	int i = 0;
	for(i = R_EAX; i <= R_EDI; i++){
		printf("%s:\t0x%x\t%d\n",regsl[i],reg_l(i),reg_l(i));
	}
	for(i = R_AX; i <= R_DI; i++){
		printf("%s:\t0x%x\t%d\n",regsw[i],reg_w(i),reg_w(i));
	}
	for(i = R_AL; i <= R_BH; i++){
		printf("%s:\t0x%x\t%d\n",regsb[i],reg_b(i),reg_b(i));
	}
}
/*Return one specific register's value*/
int info_r(char *args){
	char *arg = strtok(args,"$");
	int i = 0;
	for(i = R_EAX; i <= R_EDI; i++){
		if(strcmp(arg,regsl[i]) == 0)
			return reg_l(i);
	}
	for(i = R_AX; i <= R_DI; i++){
		if(strcmp(arg,regsw[i]) == 0)
			return reg_w(i);
	}
	for(i = R_AL; i <= R_BH; i++){
		if(strcmp(arg,regsb[i]) == 0)
			return reg_b(i);
	}
	printf("Register error\n");
	return -1;
}
/*Function for command info*/
static int cmd_info(char *args){
	char *arg = strtok(NULL, " ");
	if(strcmp(arg, "r") == 0){
		info_all_r();
	}
	if(args[0] == '$'){
		int temp = info_r(args);
		if(temp == -1)
			return 0;
		printf("%s:\t0x%x\t%d\n",args,temp,temp);
	}
	if(strcmp(arg, "w") == 0){
		print_wp();
	}
	return 0;
}

/*Function for command p*/
int cmd_p(char *args){
	bool success = true;
	int num = expr(args,&success);	
	if(success == false)
		return 0;
	printf("%d\n",num);
	return 0;
}
/*Function for command x*/
int cmd_x(char *args){
	char *num = strtok(NULL," ");
	char *arg = strtok(NULL," ");
	int i;
	bool success = true;
	int addr = expr(arg,&success);
	if(success == false)
		return 0;
	printf("0x%x: ",addr);
	for(i = 0; i < atoi(num); i++){
		printf("0x%x\t",swaddr_read(addr+i, 1));
	}
	printf("\n");
	return 0;
}
/*Function for command w*/
int cmd_w(char *args){
	set_wp(args);
	return 0;
}
/*Function for command d*/
int cmd_d(char *args){
	int num = atoi(args);
	free_wp(num);
	return 0;
}
/**/
///////////////////////////////////////////////////////
static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	/* TODO: Add more commands */
/////////////////////////////////////////////////////my code
	{ "si", "Execute the program step by step", cmd_si},
	{ "info", "Print revelent information", cmd_info},
	{ "p", "Compute a expression and return the result", cmd_p},
	{ "x", "Read the memory", cmd_x},
	{ "w", "Set watchpoint", cmd_w},
	{ "d", "Delete watchpoint", cmd_d},
//////////////////////////////////////////////////////////
	

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	init_wp_pool();
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}
		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
