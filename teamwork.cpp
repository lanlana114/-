#include <iostream>   //输入输出
#include <fstream>    //读取文件
#include <string>     //字符串
#include <vector>     //动态数组，作用：存储
#include <map>        //储存单词，次数
#include <cctype>     //处理字符
#include <windows.h>  //编码问题，加这个防止乱码
int main() {
     SetConsoleOutputCP(65001);   //VScode可能存在的编码问题，加此代码避免中文乱码

    std::string filename;
    std::cout << "请输入文件名: ";
    std::cin >> filename;

    // 1. 读取文件并按行存储
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "打开文件失败！\n";
        return 1;
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    // 2. 单词统计
    std::map<std::string, int> wordCount;
    std::ifstream file2(filename);
    if (!file2.is_open()) {
        std::cerr << "重新打开文件失败！\n";
        return 1;
    }
    std::string word;
    char ch;
    while (file2.get(ch)) {
        if (std::isalpha(ch)) {
            word.push_back(std::tolower(ch));
        } else {
            if (!word.empty()) {
                wordCount[word]++;
                word.clear();
            }
        }
    }
    if (!word.empty()) {
        wordCount[word]++;
    }
    file2.close();

    // 3. 交互菜单
    int choice;
    do {
        std::cout << "\n--- 菜单 ---\n";
        std::cout << "1. 查看原文\n";
        std::cout << "2. 查看单词统计\n";
        std::cout << "0. 退出\n";
        std::cout << "请选择: ";
        std::cin >> choice;

        if (choice == 1) {
            std::cout << "\n--- 原文 ---\n";
            for (const auto& l : lines) {
                std::cout << l << '\n';
            }
        } else if (choice == 2) {
            std::cout << "\n--- 单词统计 ---\n";
            for (const auto& p : wordCount) {
                std::cout << p.first << " : " << p.second << '\n';
            }
        } else if (choice != 0) {
            std::cout << "无效选择，请重试。\n";
        }
    } while (choice != 0);

    std::cout << "程序结束。\n";
    return 0;
}