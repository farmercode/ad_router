#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * 去掉前部的
 * @param s 传入字符串(字符数组，字符指针会报错)
 */
void ltrim(char *s)
{
    int l=0,p=0,k=0;
    l = strlen(s);
    if( l == 0 ) return;
    p = 0;
    while( s[p] == ' ' || s[p] == '\t' || s[p] == '\n' || s[p] == '\r' || s[p] == '"')  p++;
    if( p == 0 ) return;
    while( s[k] != '\0') s[k++] = s[p++];
    return;
}

/**
 * 去掉尾部的
 * @param s 传入字符串(字符数组，字符指针会报错)
 */
void rtrim(char *s)
{
    int l=0,p=0;
    l = strlen(s);
    if( l == 0 ) return;
    p = l -1;
    while( s[p] == ' ' || s[p] == '\t' || s[p] == '\n' || s[p] == '\r'|| s[p] == '"' ) {
        s[p--] = '\0';
        if( p < 0 ) break;
    }
    return;
}

/**
 * 判断字符串是否全为数字
 * @param  str [description]
 * @return     [description]
 */
int is_number(char *str){
  while(*str) {
    if(!isdigit(*str++))
      return 0;
  }
  return 1;
}

int compare_string(char *str1,char *str2){
    if(!strncasecmp(str1,str2,strlen(str2))){
        return 1;
    }else{
        return 0;
    }
}

/**
 * 去除字符串中注释
 * 注释符号为#
 * @param str [description]
 */
void remove_config_note(char *str){
    int index=0;
    char *tmp;
    tmp = (char *)malloc(strlen(str)*sizeof(char));

    while(str[index] != '\0'){
        if(str[index] == '#') break;
        tmp[index] = str[index];
        index++;
    }
    strcpy(str,tmp);
    free(tmp);
}


char *urlencode(char* str)
{
    int i=0,j=0,str_len;
    char ch;
    str_len = strlen(str);
    char *encode_str;

    encode_str = (char *)malloc((str_len*3+1)*sizeof(char));
    

    for ( ; i<str_len; i++) {
        ch = str[i];
        if (((ch>='A') && (ch<'Z')) ||
            ((ch>='a') && (ch<'z')) ||
            ((ch>='0') && (ch<'9'))) {
            encode_str[j++] = ch;
        } else if (ch == ' ') {
            encode_str[j++] = '+';
        } else if (ch == '.' || ch == '-' || ch == '_' || ch == '*') {
            encode_str[j++] = ch;
        } else {
                sprintf(encode_str+j, "%%%02X", (unsigned char)ch);
                j += 3;
        }
    }

    encode_str[j] = '\0';  
    return encode_str;  
}  

void urldecode(char *p){
    int i=0;
    while(*(p+i)){
        if ((*p=*(p+i)) == '%'){
            *p=*(p+i+1) >= 'A' ? ((*(p+i+1) & 0XDF) - 'A') + 10 : (*(p+i+1) - '0');
            *p=(*p) * 16;
            *p+=*(p+i+2) >= 'A' ? ((*(p+i+2) & 0XDF) - 'A') + 10 : (*(p+i+2) - '0');
            i+=2;
        }else if (*(p+i)=='+'){
            *p=' ';
        }
        p++;
    }
    *p='\0';
}


void trim_yinhao(char *s)
{
    int l=0,p=0,k=0;
    l = strlen(s);
    if( l == 0 ) return;
    p = 0;
    while( s[p] == '"' )  p++;
    if( p == 0 ) return;
    while( s[k] != '\0'){
        if(s[k] == '"'){
            break;
        }
        s[k++] = s[p++];
    } 
    return;
}