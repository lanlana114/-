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

// Huffman 二叉树节点，用于文本压缩/解压
typedef struct HuffmanNode {
    unsigned char ch;
    int freq;
    struct HuffmanNode* left;
    struct HuffmanNode* right;
} HuffmanNode;

// 创建 Huffman 节点
HuffmanNode* create_huffman_node(unsigned char ch, int freq) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    if (!node) {
        return NULL;
    }
    node->ch = ch;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

// 释放 Huffman 树内存
void free_huffman_tree(HuffmanNode* root) {
    if (!root) {
        return;
    }
    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}

// 构建 Huffman 树
HuffmanNode* build_huffman_tree(const int freq[256]) {
    HuffmanNode* nodes[256];
    int nodeCount = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            nodes[nodeCount++] = create_huffman_node((unsigned char)i, freq[i]);
        }
    }
    if (nodeCount == 0) {
        return NULL;
    }
    while (nodeCount > 1) {
        int min1 = -1, min2 = -1;
        for (int i = 0; i < nodeCount; i++) {
            if (min1 == -1 || nodes[i]->freq < nodes[min1]->freq) {
                min2 = min1;
                min1 = i;
            } else if (min2 == -1 || nodes[i]->freq < nodes[min2]->freq) {
                min2 = i;
            }
        }
        HuffmanNode* left = nodes[min1];
        HuffmanNode* right = nodes[min2];
        HuffmanNode* parent = create_huffman_node(0, left->freq + right->freq);
        if (!parent) {
            return NULL;
        }
        parent->left = left;
        parent->right = right;
        if (min1 > min2) {
            int tmp = min1;
            min1 = min2;
            min2 = tmp;
        }
        nodes[min1] = parent;
        nodes[min2] = nodes[nodeCount - 1];
        nodeCount--;
    }
    return nodes[0];
}

// 生成每个字符的 Huffman 码
void build_huffman_codes(HuffmanNode* root, char codes[256][MAX_CODE_LEN], char code[], int depth) {
    if (!root) {
        return;
    }
    if (!root->left && !root->right) {
        if (depth == 0) {
            code[depth++] = '0';
        }
        code[depth] = '\0';
        strcpy(codes[root->ch], code);
        return;
    }
    if (root->left) {
        code[depth] = '0';
        build_huffman_codes(root->left, codes, code, depth + 1);
    }
    if (root->right) {
        code[depth] = '1';
        build_huffman_codes(root->right, codes, code, depth + 1);
    }
}

// 统计非零符号数量
int count_unique_symbols(const int freq[256]) {
    int count = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            count++;
        }
    }
    return count;
}

// 写单个位到输出文件
int write_bit(FILE* out, unsigned char bit, unsigned char* bitBuffer, int* bitCount) {
    *bitBuffer = (unsigned char)((*bitBuffer << 1) | (bit & 1));
    (*bitCount)++;
    if (*bitCount == 8) {
        if (fwrite(bitBuffer, 1, 1, out) != 1) {
            return 0;
        }
        *bitBuffer = 0;
        *bitCount = 0;
    }
    return 1;
}

// 刷新剩余位
int flush_bit_buffer(FILE* out, unsigned char* bitBuffer, int* bitCount) {
    if (*bitCount == 0) {
        return 1;
    }
    *bitBuffer <<= (8 - *bitCount);
    if (fwrite(bitBuffer, 1, 1, out) != 1) {
        return 0;
    }
    *bitBuffer = 0;
    *bitCount = 0;
    return 1;
}

// 压缩文件
int compress_file(const char* input, const char* output) {
    FILE* in = fopen(input, "rb");
    if (!in) {
        return 0;
    }

    int freq[256] = {0};
    uint64_t total = 0;
    int ch;
    while ((ch = fgetc(in)) != EOF) {
        freq[(unsigned char)ch]++;
        total++;
    }
    if (total == 0) {
        fclose(in);
        return 0;
    }
    HuffmanNode* root = build_huffman_tree(freq);
    if (!root) {
        fclose(in);
        return 0;
    }

    char codes[256][MAX_CODE_LEN] = {{0}};
    char code[MAX_CODE_LEN];
    build_huffman_codes(root, codes, code, 0);

    FILE* out = fopen(output, "wb");
    if (!out) {
        free_huffman_tree(root);
        fclose(in);
        return 0;
    }

    fwrite("HTF1", 1, 4, out);
    uint64_t size = total;
    uint32_t uniqueCount = (uint32_t)count_unique_symbols(freq);
    fwrite(&size, sizeof(size), 1, out);
    fwrite(&uniqueCount, sizeof(uniqueCount), 1, out);
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            unsigned char symbol = (unsigned char)i;
            fwrite(&symbol, 1, 1, out);
            uint64_t f = (uint64_t)freq[i];
            fwrite(&f, sizeof(f), 1, out);
        }
    }

    rewind(in);
    unsigned char bitBuffer = 0;
    int bitCount = 0;
    while ((ch = fgetc(in)) != EOF) {
        const char* codeStr = codes[(unsigned char)ch];
        for (int i = 0; codeStr[i] != '\0'; i++) {
            unsigned char bit = (unsigned char)(codeStr[i] == '1');
            if (!write_bit(out, bit, &bitBuffer, &bitCount)) {
                fclose(in);
                fclose(out);
                free_huffman_tree(root);
                return 0;
            }
        }
    }
    flush_bit_buffer(out, &bitBuffer, &bitCount);
    fclose(in);
    fclose(out);
    free_huffman_tree(root);
    return 1;
}

// 读取文件中的单个位
int read_bit(unsigned char byte, int position) {
    return (byte >> (7 - position)) & 1;
}

// 解压文件
int decompress_file(const char* input, const char* output) {
    FILE* in = fopen(input, "rb");
    if (!in) {
        return 0;
    }
    char magic[4];
    if (fread(magic, 1, 4, in) != 4 || strncmp(magic, "HTF1", 4) != 0) {
        fclose(in);
        return 0;
    }
    uint64_t originalSize = 0;
    if (fread(&originalSize, sizeof(originalSize), 1, in) != 1) {
        fclose(in);
        return 0;
    }
    uint32_t uniqueCount = 0;
    if (fread(&uniqueCount, sizeof(uniqueCount), 1, in) != 1) {
        fclose(in);
        return 0;
    }
    int freq[256] = {0};
    for (uint32_t i = 0; i < uniqueCount; i++) {
        unsigned char symbol;
        uint64_t f;
        if (fread(&symbol, 1, 1, in) != 1 || fread(&f, sizeof(f), 1, in) != 1) {
            fclose(in);
            return 0;
        }
        freq[symbol] = (int)f;
    }
    HuffmanNode* root = build_huffman_tree(freq);
    if (!root) {
        fclose(in);
        return 0;
    }
    FILE* out = fopen(output, "wb");
    if (!out) {
        free_huffman_tree(root);
        fclose(in);
        return 0;
    }

    if (!root->left && !root->right) {
        unsigned char symbol = root->ch;
        for (uint64_t i = 0; i < originalSize; i++) {
            fwrite(&symbol, 1, 1, out);
        }
        fclose(in);
        fclose(out);
        free_huffman_tree(root);
        return 1;
    }

    HuffmanNode* node = root;
    int bytesRead;
    unsigned char byte;
    uint64_t written = 0;
    while (written < originalSize && fread(&byte, 1, 1, in) == 1) {
        for (int bitPos = 0; bitPos < 8 && written < originalSize; bitPos++) {
            int bit = read_bit(byte, bitPos);
            node = bit == 0 ? node->left : node->right;
            if (!node) {
                fclose(in);
                fclose(out);
                free_huffman_tree(root);
                return 0;
            }
            if (!node->left && !node->right) {
                fwrite(&node->ch, 1, 1, out);
                written++;
                node = root;
                if (written >= originalSize) {
                    break;
                }
            }
        }
    }
    fclose(in);
    fclose(out);
    free_huffman_tree(root);
    return written == originalSize;
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
        printf("6. 关键词查找 (朴素/KMP)\n");
        printf("7. 单词字典序排序输出\n");
        printf("8. 文本压缩/解压\n");
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

        // 处理用户选择
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
        } else if (choice == 6) {
            // 关键词查找
            int mode = 0;
            char keyword[MAX_LINE_LEN];
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            printf("请输入关键词: ");
            if (fgets(keyword, sizeof(keyword), stdin) == NULL) {
                printf("读取关键词失败。\n");
                continue;
            }
            size_t keylen = strlen(keyword);
            if (keylen > 0 && keyword[keylen - 1] == '\n') {
                keyword[keylen - 1] = '\0';
            }
            if (keyword[0] == '\0') {
                printf("关键词不能为空。\n");
                continue;
            }
            printf("请选择查找模式: 1=朴素匹配, 2=KMP匹配, 3=对比两者效率: ");
            if (scanf("%d", &mode) != 1 || mode < 1 || mode > 3) {
                while ((ch = getchar()) != '\n' && ch != EOF) {
                }
                printf("无效选择，返回菜单。\n");
                continue;
            }
            if (mode == 1 || mode == 2) {
                clock_t start = clock();
                search_keyword(lines, lineCount, keyword, mode == 2, 1);
                clock_t end = clock();
                printf("模式 %s 用时: %.3f 毫秒\n", mode == 1 ? "朴素匹配" : "KMP匹配",
                       (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
            } else {
                clock_t start1 = clock();
                int count1 = search_keyword(lines, lineCount, keyword, 0, 0);
                clock_t end1 = clock();
                double time1 = (double)(end1 - start1) * 1000.0 / CLOCKS_PER_SEC;
                clock_t start2 = clock();
                int count2 = search_keyword(lines, lineCount, keyword, 1, 0);
                clock_t end2 = clock();
                double time2 = (double)(end2 - start2) * 1000.0 / CLOCKS_PER_SEC;
                printf("\n对比结果:\n");
                printf("朴素匹配 结果数量: %d, 用时: %.3f 毫秒\n", count1, time1);
                printf("KMP匹配 结果数量: %d, 用时: %.3f 毫秒\n", count2, time2);
                if (count1 != count2) {
                    printf("警告：两种匹配结果数量不同，请检查关键词或文本。\n");
                } else {
                    printf("结果数量一致。\n");
                }
                if (time1 < time2) {
                    printf("朴素匹配更快。\n");
                } else if (time1 > time2) {
                    printf("KMP匹配更快。\n");
                } else {
                    printf("两种匹配耗时相同。\n");
                }
                if (count1 > 0) {
                    printf("\n使用朴素匹配打印位置：\n");
                    search_keyword(lines, lineCount, keyword, 0, 1);
                }
            }
        } else if (choice == 7) {
            char sortFilename[512];
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            printf("请输入文本文件名: ");
            if (fgets(sortFilename, sizeof(sortFilename), stdin) == NULL) {
                printf("读取文件名失败。\n");
                continue;
            }
            size_t len = strlen(sortFilename);
            if (len > 0 && sortFilename[len - 1] == '\n') {
                sortFilename[len - 1] = '\0';
            }
            if (sortFilename[0] == '\0') {
                printf("文件名不能为空。\n");
                continue;
            }
            show_sorted_words(sortFilename);
        } else if (choice == 8) {
            int action = 0;
            char inputName[512];
            char outputName[512];
            size_t len;
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            printf("请选择操作: 1=压缩文件, 2=解压文件: ");
            if (scanf("%d", &action) != 1 || (action != 1 && action != 2)) {
                while ((ch = getchar()) != '\n' && ch != EOF) {
                }
                printf("无效选择，返回菜单。\n");
                continue;
            }
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            if (action == 1) {
                printf("请输入要压缩的文本文件名: ");
                if (fgets(inputName, sizeof(inputName), stdin) == NULL) {
                    printf("读取文件名失败。\n");
                    continue;
                }
                len = strlen(inputName);
                if (len > 0 && inputName[len - 1] == '\n') {
                    inputName[len - 1] = '\0';
                }
                printf("请输入输出压缩文件名: ");
                if (fgets(outputName, sizeof(outputName), stdin) == NULL) {
                    printf("读取文件名失败。\n");
                    continue;
                }
                len = strlen(outputName);
                if (len > 0 && outputName[len - 1] == '\n') {
                    outputName[len - 1] = '\0';
                }
                if (compress_file(inputName, outputName)) {
                    printf("压缩成功: %s -> %s\n", inputName, outputName);
                } else {
                    printf("压缩失败，请检查文件名或文件内容。\n");
                }
            } else {
                printf("请输入要解压的压缩文件名: ");
                if (fgets(inputName, sizeof(inputName), stdin) == NULL) {
                    printf("读取文件名失败。\n");
                    continue;
                }
                len = strlen(inputName);
                if (len > 0 && inputName[len - 1] == '\n') {
                    inputName[len - 1] = '\0';
                }
                printf("请输入输出文本文件名: ");
                if (fgets(outputName, sizeof(outputName), stdin) == NULL) {
                    printf("读取文件名失败。\n");
                    continue;
                }
                len = strlen(outputName);
                if (len > 0 && outputName[len - 1] == '\n') {
                    outputName[len - 1] = '\0';
                }
                if (decompress_file(inputName, outputName)) {
                    printf("解压成功: %s -> %s\n", inputName, outputName);
                } else {
                    printf("解压失败，请检查压缩文件是否有效。\n");
                }
            }
        } else if (choice != 0) {
            printf("无效选择，请重试。\n");
        }
    } while (choice != 0);

    printf("程序结束。\n");
    return 0;
}
