 #include <stdio.h>   
#include <string>     //字符串
#include <cctype>     //处理字符
#include <windows.h>  //编码问题，加这个防止乱码

#define MAX_LINES 1000  //最大行数
#define MAX_LINE_LEN 1024  //最大行长度
#define MAX_WORDS 2000  //最大单词数
#define MAX_WORD_LEN 64  //最大单词长度
#define MAX_TASKS 50    //最大批处理任务数

int is_matching_pair(char left, char right)  {
    return (left == '(' && right == ')') 
        || (left == '(' && right == ')') 
        || (left == '{' && right == '}') ;

}

int check_brackets(const char* text) {
(void)text;
    return 1;
}

typedef struct {
    char word[MAX_WORD_LEN];  //单词
    int count;                //出现次数
} WordItem;

typedef struct {
    char filename[512];
    int action; // 1=查看原文, 2=单词统计
} Task;

int find_word(WordItem wordCount[], int size, const char* word) {
    for (int i = 0; i < size; i++) {
        if (strcmp(wordCount[i].word, word) == 0) {
            return i;  //找到单词，返回索引
        }
    }
    return -1;  //未找到单词
}

int read_lines(const char* filename, char lines[][MAX_LINE_LEN], int* outLineCount) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return 0;
    }
    int lineCount = 0;
    while (lineCount < MAX_LINES && fgets(lines[lineCount], MAX_LINE_LEN, file) != NULL) {
        lineCount++;
    }
    fclose(file);
    *outLineCount = lineCount;
    return 1;
}

int count_words(const char* filename, WordItem wordCount[], int* outCount) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return 0;
    }

    int wordCountSize = 0;
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
    *outCount = wordCountSize;
    return 1;
}

void show_text(const char* filename) {
    static char lines[MAX_LINES][MAX_LINE_LEN];
    int lineCount = 0;
    if (!read_lines(filename, lines, &lineCount)) {
        printf("无法打开文件: %s\n", filename);
        return;
    }
    printf("\n--- 原文: %s ---\n", filename);
    for (int i = 0; i < lineCount; i++) {
        printf("%s", lines[i]);
    }
    printf("\n");
}

void show_word_count(const char* filename) {
    WordItem wordCount[MAX_WORDS];
    int wordCountSize = 0;
    if (!count_words(filename, wordCount, &wordCountSize)) {
        printf("无法打开文件: %s\n", filename);
        return;
    }
    printf("\n--- 单词统计: %s ---\n", filename);
    for (int i = 0; i < wordCountSize; i++) {
        printf("%s : %d\n", wordCount[i].word, wordCount[i].count);
    }
    if (wordCountSize == 0) {
        printf("(未找到单词)\n");
    }
    printf("\n");
}

void print_task(const Task* task, int index) {
    printf("%d. 文件名: %s, 类型: %s\n", index + 1, task->filename,
           task->action == 1 ? "查看原文" : "单词统计");
}

void execute_batch(Task tasks[], int taskCount) {
    if (taskCount == 0) {
        printf("当前没有批处理任务。\n");
        return;
    }
    printf("\n开始执行批处理任务，共 %d 个任务。\n", taskCount);
    for (int i = 0; i < taskCount; i++) {
        printf("\n[任务 %d/%d] %s - %s\n", i + 1, taskCount,
               tasks[i].filename,
               tasks[i].action == 1 ? "查看原文" : "单词统计");
        if (tasks[i].action == 1) {
            show_text(tasks[i].filename);
        } else {
            show_word_count(tasks[i].filename);
        }
    }
    printf("批处理执行完成。\n");
}

int main(void) {
    SetConsoleOutputCP(65001);

    char filename[512];
    printf("请输入文件名: ");
     if (fgets(filename, sizeof(filename), stdin) == NULL) {
            printf("读取文件名失败。\n");
            return 1;
        }
        size_t len = strlen(filename);
        if (len > 0 && filename[len - 1] == '\n') {
            filename[len - 1] = '\0';
        }
        if (filename[0] == '\0') {
            printf("文件名不能为空。\n");
            continue;
        }

     file = fopen(filename, "r");
        if (file) {
            break;
        }
        printf("无法打开文件：%s\n请检查路径后重新输入。\n", filename);
    }


    static char lines[MAX_LINES][MAX_LINE_LEN];
    int lineCount = 0;
WordItem wordCount[MAX_WORDS];
    int wordCountSize = 0;

char word[MAX_WORD_LEN];
    int wi = 0;
    int c;

while (lineCount < MAX_LINES && fgets(lines[lineCount], MAX_LINE_LEN, file) != NULL) {
        char *p = lines[lineCount];
        while (*p) {
            c = (unsigned char)*p;
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
            p++;
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
            wi = 0;
        }
        lineCount++;
    }
    fclose(file);

    Task tasks[MAX_TASKS];
    int taskCount = 0;
    int choice;
    do {
        printf("\n--- 菜单 ---\n");
        printf("1. 添加批处理任务\n");
        printf("2. 查看批处理队列\n");
        printf("3. 执行批处理任务\n");
        printf("4. 查看原文\n");
        printf("5. 查看单词统计\n");
        printf("0. 退出\n");
        printf("请选择: ");
        if (scanf("%d", &choice) != 1) {
           int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            printf("输入无效，请输入数字。\n");
            choice = -1;
            continue;
        }

        if (choice == 1) {
          char taskFilename[512];
            int action = 0;
            printf("请输入文本文件名: ");
            if (scanf("%511s", taskFilename) != 1) {
                printf("读取文件名失败。\n");
                continue;
            }
            printf("请选择处理类型: 1=查看原文, 2=单词统计: ");
            if (scanf("%d", &action) != 1 || (action != 1 && action != 2)) {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF) {
                }
                printf("无效选择，任务未添加。\n");
                continue;
            }
            strcpy(tasks[taskCount].filename, taskFilename);
            tasks[taskCount].action = action;
            taskCount++;
            printf("已添加批处理任务: %s (%s)\n", taskFilename,
                   action == 1 ? "查看原文" : "单词统计");
        } else if (choice == 2) {
            if (taskCount == 0) {
                printf("当前没有批处理任务。\n");
            } else {
                printf("\n当前批处理队列 (%d 个任务):\n", taskCount);
                for (int i = 0; i < taskCount; i++) {
                    print_task(&tasks[i], i);
                }
            }
        } else if (choice == 3) {
            execute_batch(tasks, taskCount);
            taskCount = 0;
        } else if (choice == 4) {
            printf("\n--- 原文 ---\n");
            for (int i = 0; i < lineCount; i++) {
                printf("%s", lines[i]);
            }
        } else if (choice == 5) {
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
