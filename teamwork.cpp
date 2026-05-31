#include <stdio.h>
#include <string.h>   //字符串处理
#include <ctype.h>    //处理字符
#include <windows.h>  //编码问题，加这个防止乱码
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define MAX_LINES 1000  //最大行数
#define MAX_LINE_LEN 1024  //最大行长度
#define MAX_WORDS 2000  //最大单词数
#define MAX_WORD_LEN 64  //最大单词长度
#define MAX_TASKS 50    //最大批处理任务数
#define MAX_CODE_LEN 512 // 哈夫曼编码最大长度

// 检查左右括号是否匹配的函数
int is_matching_pair(char left, char right) {
    return (left == '(' && right == ')')
        || (left == '[' && right == ']')
        || (left == '{' && right == '}');
}
// 检查文本中括号是否平衡的函数（目前未实现，返回1）
int check_brackets(const char* text) {
    (void)text;
    return 1;
}
// 定义单词项结构体，包含单词和出现次数
typedef struct {
    char word[MAX_WORD_LEN];  //单词
    int count;                //出现次数
} WordItem;
// 定义任务结构体，包含文件名和操作类型
typedef struct {
    char filename[512];
    int action; // 1=查看原文, 2=单词统计
} Task;
// 在单词数组中查找单词的函数
int find_word(WordItem wordCount[], int size, const char* word) {
    for (int i = 0; i < size; i++) {
        if (strcmp(wordCount[i].word, word) == 0) {
            return i;  //找到单词，返回索引
        }
    }
    return -1;  //未找到单词
}
// 从文件中读取行的函数
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
// 统计文件中单词出现次数的函数
int count_words(const char* filename, WordItem wordCount[], int* outCount) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return 0;
    }

    int wordCountSize = 0;
    char word[MAX_WORD_LEN];
    int wi = 0;
    int c;
// 逐字符读取文件，提取单词并统计
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
 // 处理最后一个单词
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
// 显示文件原文的函数
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
// 显示单词统计结果的函数
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
// 二叉搜索树节点，用于按字典序排序单词
typedef struct WordNode {
    char word[MAX_WORD_LEN];
    int count;
    struct WordNode* left;
    struct WordNode* right;
} WordNode;

// 创建单词节点
WordNode* create_word_node(const char* word) {
    WordNode* node = (WordNode*)malloc(sizeof(WordNode));
    if (!node) {
        return NULL;
    }
    strcpy(node->word, word);
    node->count = 1;
    node->left = node->right = NULL;
    return node;
}

// 插入单词到二叉搜索树，如果已存在则增加计数
WordNode* insert_word_node(WordNode* root, const char* word) {
    if (!root) {
        return create_word_node(word);
    }
    int cmp = strcmp(word, root->word);
    if (cmp == 0) {
        root->count++;
    } else if (cmp < 0) {
        root->left = insert_word_node(root->left, word);
    } else {
        root->right = insert_word_node(root->right, word);
    }
    return root;
}

// 中序遍历并输出单词
void inorder_print_word_tree(const WordNode* root) {
    if (!root) {
        return;
    }
    inorder_print_word_tree(root->left);
    printf("%s : %d\n", root->word, root->count);
    inorder_print_word_tree(root->right);
}

// 释放单词树内存
void free_word_tree(WordNode* root) {
    if (!root) {
        return;
    }
    free_word_tree(root->left);
    free_word_tree(root->right);
    free(root);
}

// 从文件读取单词并构建二叉搜索树
int build_word_bst(const char* filename, WordNode** rootOut) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return 0;
    }
    *rootOut = NULL;
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
                *rootOut = insert_word_node(*rootOut, word);
                wi = 0;
            }
        }
    }
    if (wi > 0) {
        word[wi] = '\0';
        *rootOut = insert_word_node(*rootOut, word);
    }
    fclose(file);
    return 1;
}

// 按字典序输出文本文件中所有单词及其统计
void show_sorted_words(const char* filename) {
    WordNode* root = NULL;
    if (!build_word_bst(filename, &root)) {
        printf("无法打开文件: %s\n", filename);
        return;
    }
    printf("\n--- 按字典序排序的单词: %s ---\n", filename);
    if (!root) {
        printf("(未找到单词)\n\n");
        return;
    }
    inorder_print_word_tree(root);
    printf("\n");
    free_word_tree(root);
}
// 计算 KMP 模式函数的最长前缀后缀数组
void compute_lps(const char* pattern, int m, int lps[]) {
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

// 朴素字符串匹配，仅使用数组和字符比较
int naive_search_line(const char* line, const char* pattern, int positions[], int maxPositions) {
    int n = strlen(line);
    int m = strlen(pattern);
    int found = 0;
    if (m == 0 || n < m) {
        return 0;
    }
    for (int i = 0; i <= n - m; i++) {
        int j;
        for (j = 0; j < m; j++) {
            if (line[i + j] != pattern[j]) {
                break;
            }
        }
        if (j == m) {
            if (found < maxPositions) {
                positions[found] = i;
            }
            found++;
        }
    }
    return found;
}
// KMP 字符串匹配
int kmp_search_line(const char* line, const char* pattern, int positions[], int maxPositions) {
    int n = strlen(line);
    int m = strlen(pattern);
    int found = 0;
    if (m == 0 || n < m) {
        return 0;
    }
    int* lps = (int*)malloc(sizeof(int) * m);
    if (!lps) {
        return 0;
    }
    compute_lps(pattern, m, lps);
    int i = 0;
    int j = 0;
    while (i < n) {
        if (line[i] == pattern[j]) {
            i++;
            j++;
            if (j == m) {
                if (found < maxPositions) {
                    positions[found] = i - j;
                }
                found++;
                j = lps[j - 1];
            }
        } else {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }
    free(lps);
    return found;
}

// 执行关键词查找，返回匹配总数，可选择是否打印位置
int search_keyword(const char lines[][MAX_LINE_LEN], int lineCount, const char* keyword, int useKMP, int verbose) {
    const int maxPositionsPerLine = 1024;
    int positions[maxPositionsPerLine];
    int totalCount = 0;
    for (int i = 0; i < lineCount; i++) {
        int count = useKMP ? kmp_search_line(lines[i], keyword, positions, maxPositionsPerLine)
                           : naive_search_line(lines[i], keyword, positions, maxPositionsPerLine);
        if (verbose) {
            for (int k = 0; k < count && k < maxPositionsPerLine; k++) {
                printf("第 %d 行，第 %d 列\n", i + 1, positions[k] + 1);
            }
        }
        totalCount += count;
    }
    if (verbose) {
        if (totalCount == 0) {
            printf("未找到关键词 '%s'。\n", keyword);
        } else {
            printf("共找到 %d 处匹配。\n", totalCount);
        }
    }
    return totalCount;
}


// 打印任务信息的函数
void print_task(const Task* task, int index) {
    printf("%d. 文件名: %s, 类型: %s\n", index + 1, task->filename,
           task->action == 1 ? "查看原文" : "单词统计");
}
// 执行批处理任务的函数
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
    SetConsoleOutputCP(65001); // 设置控制台输出编码为UTF-8，防止中文乱码

    char filename[512];
    FILE *file = NULL;// 循环提示用户输入文件名，直到成功打开文件
    while (1) {
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
// 初始化变量用于存储文件内容和单词统计
    static char lines[MAX_LINES][MAX_LINE_LEN];
    int lineCount = 0;
    WordItem wordCount[MAX_WORDS];
    int wordCountSize = 0;

    char word[MAX_WORD_LEN];
    int wi = 0;
    int c;
 // 读取文件内容并同时进行单词统计
    while (lineCount < MAX_LINES && fgets(lines[lineCount], MAX_LINE_LEN, file) != NULL) {
        char *p = lines[lineCount];
         // 逐字符处理当前行，提取单词
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
         // 处理行末的单词
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
  // 初始化批处理任务数组
    Task tasks[MAX_TASKS];
    int taskCount = 0;
    int choice;
     // 主菜单循环
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
            // 添加批处理任务
            if (taskCount >= MAX_TASKS) {
                printf("任务队列已满，无法添加更多任务。\n");
                continue;
            }
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
            // 查看批处理队列
            if (taskCount == 0) {
                printf("当前没有批处理任务。\n");
            } else {
                printf("\n当前批处理队列 (%d 个任务):\n", taskCount);
                for (int i = 0; i < taskCount; i++) {
                    print_task(&tasks[i], i);
                }
            }
        } else if (choice == 3) {
             // 执行批处理任务
            execute_batch(tasks, taskCount);
            taskCount = 0;
        } else if (choice == 4) {
            // 查看当前文件的原文
            printf("\n--- 原文 ---\n");
            for (int i = 0; i < lineCount; i++) {
                printf("%s", lines[i]);
            }
        } else if (choice == 5) {
            // 查看当前文件的单词统计
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
