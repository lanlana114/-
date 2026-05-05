 #include <stdio.h>   
#include <string>     //字符串
#include <cctype>     //处理字符
#include <windows.h>  //编码问题，加这个防止乱码

#define MAX_LINES 1000  //最大行数
#define MAX_LINE_LEN 1024  //最大行长度
#define MAX_WORDS 2000  //最大单词数
#define MAX_WORD_LEN 64  //最大单词长度

typedef struct {
    char word[MAX_WORD_LEN];  //单词
    int count;                //出现次数
} WordItem;

int find_word(WordItem wordCount[], int size, const char* word) {
    for (int i = 0; i < size; i++) {
        if (strcmp(wordCount[i].word, word) == 0) {
            return i;  //找到单词，返回索引
        }
    }
    return -1;  //未找到单词
}

int main(void) {
    SetConsoleOutputCP(65001);

    char filename[512];
    printf("请输入文件名: ");
    if (scanf("%511s", filename) != 1) {
        printf("读取文件名失败。\n");
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("打开文件失败！\n");
        return 1;
    }

    static char lines[MAX_LINES][MAX_LINE_LEN];
    int lineCount = 0;
    while (lineCount < MAX_LINES && fgets(lines[lineCount], MAX_LINE_LEN, file) != NULL) {
        lineCount++;
    }
    fclose(file);

    WordItem wordCount[MAX_WORDS];
    int wordCountSize = 0;

    file = fopen(filename, "r");
    if (!file) {
        printf("打开文件失败！\n");
        return 1;
    }
    char word[MAX_WORD_LEN];
    int wi = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (isalpha(c)) {
            if (wi < MAX_WORD_LEN - 1) {
                word[wi++] = (char)tolower(c);
            }
        } else {
            if (wi > 0) {
                word[wi] = '\0';
                int idx = find_word(wordCount, wordCountSize, word);
                if (idx >= 0) {
                    wordCount[idx].count++;
                } else if (wordCountSize < MAX_WORDS) {
                    strcpy(wordCount[wordCountSize].word, word);
                    wordCount[wordCountSize].count = 1;
                    wordCountSize++;
                }
                wi = 0;
            }
        }
    }
    if (wi > 0) {
        word[wi] = '\0';
        int idx = find_word(wordCount, wordCountSize, word);
        if (idx >= 0) {
            wordCount[idx].count++;
        } else if (wordCountSize < MAX_WORDS) {
            strcpy(wordCount[wordCountSize].word, word);
            wordCount[wordCountSize].count = 1;
            wordCountSize++;
        }
    }
    fclose(file);

    int choice;
    do {
        printf("\n--- 菜单 ---\n");
        printf("1. 查看原文\n");
        printf("2. 查看单词统计\n");
        printf("0. 退出\n");
        printf("请选择: ");
        if (scanf("%d", &choice) != 1) {
            break;
        }

        if (choice == 1) {
            printf("\n--- 原文 ---\n");
            for (int i = 0; i < lineCount; i++) {
                printf("%s", lines[i]);
            }
        } else if (choice == 2) {
            printf("\n--- 单词统计 ---\n");
            for (int i = 0; i < wordCountSize; i++) {
                printf("%s : %d\n", wordCount[i].word, wordCount[i].count);
            }
        } else if (choice != 0) {
            printf("无效选择，请重试。\n");
        }
    } while (choice != 0);

    printf("程序结束。\n");
    return 0;
}
