#include "nv.h"

#define NV_ENV_TERM_STDC
//#define NV_ENV_TERM_UNIX


#ifdef NV_ENV_TERM_STDC

char *NV_gets(char *str, int size)
{
	return fgets(str, size, stdin);
}

#endif

#ifdef NV_ENV_TERM_UNIX

#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

struct termios base_conf;
int flg_set_atexit;

void atexit_recover_termios(void)
{
	tcsetattr(0 , TCSAFLUSH , &base_conf );
}

char *NV_gets(char *str, int size)
{
	int c;
	struct termios io_conf;
	int useLen = 0, p = 0, i, movs;

	tcgetattr( 0 , &io_conf );
	base_conf = io_conf;

	if(!flg_set_atexit){
		atexit(atexit_recover_termios);
		flg_set_atexit = 1;
	}

	io_conf.c_lflag &= ~( ECHO | ICANON );
	io_conf.c_cc[VMIN]  = 1;
	io_conf.c_cc[VTIME] = 0;
	tcsetattr( 0 , TCSAFLUSH , &io_conf );
	
	while(1){
		c = getchar();
		if(c == EOF) break;
		else if(c == '\n'){
			printf("\x1b[S");
			printf("\x1b[E");
			printf("\n");
			break;
		} else if(c == 0x04){	// EOT
			break;
		} else if(c == 0x08 || c == 0x7f){
			// backspace
			if(p <= 0) continue;
			printf("\x1b[D\x1b[J");	// move left and clear chars after cursor.
			p--;
			useLen--;
			for(i = p; i < useLen; i++){
				str[i] = str[i + 1];
				fputc(str[i], stdout);
			}
			movs = useLen - p;
			for(i = 0; i < movs; i++){
				printf("\x1b[D");
			}
			continue;
		} else if(c == 0x1b){
			c = getchar();
			if(c == 0x5b){
				c = getchar();
				if(c == 0x44){
					if(p > 0){
						printf("\x1b[D");	// move cursor to left
						p--;
					}
				} else if(c == 0x43){
					if(p < useLen){
						printf("\x1b[C");	// move cursor to right
						p++;
					}
				}
			}
		} else if(0x20 <= c && c <= 0x7e){
			if(useLen >= size - 1) continue;
			printf("\x1b[J");
			fputc(c, stdout);
			for(i = useLen; i > p; i--){
				str[i] = str[i - 1];
			}
			for(i = p + 1; i <= useLen; i++){
				fputc(str[i], stdout);
			}
			movs = useLen - p;
			for(i = 0; i < movs; i++){
				printf("\x1b[D");
			}
			str[p] = c;
			useLen++;
			p++;
		} else{
			//printf("0x%d", c);
		}
	}
	tcsetattr(0 , TCSAFLUSH , &base_conf );
	
	if(c == EOF && useLen == 0) return NULL;
	if(c == 0x04) return NULL;
	str[useLen] = 0;
	return str;
}

#endif
