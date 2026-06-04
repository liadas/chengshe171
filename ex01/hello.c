#include <stdio.h>
int main(){
    printf("Hello,World!\n");
    //新增
    char buf[128];
    printf("请输入任意文字：");
    fgets(buf, sizeof(buf), stdin);
    printf("你输入的内容：%s", buf);

    return 0;
}