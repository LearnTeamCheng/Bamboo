#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <initializer_list>
#include <filesystem>
#include <string_view>

namespace Bamboo
{

    class FileUtlis
    {
    public:
        //@brief 检测文件是否存在
        static bool FileExist(const std::string &filePath);
        //@brief 获取路径下的所有文件列表 @param path 路径 @param recursive 是否递归
        static  std::vector<std::filesystem::path> GetFiles(const std::filesystem::path& path, bool recursive = false);
        //@brief 是否是目录
        static bool IsDirector(const std::string &path);
        //@brief 创建目录
        static bool CreateDirectory(const std::string &path);
        //@brief 删除目录
        static bool RemoveDirectory(const std::string &path);
        //@brief 删除文件
        static bool RemoveFile(const std::string &path);
        //@brief 复制文件
        static bool CopyFile(const std::string &src, const std::string &dst);
        //@brief 移动文件
        static bool MoveFile(const std::string &src, const std::string &dst);
        //@brief 重命名文件
        static void RenameFile(const std::string &src, const std::string &dst);

        //@brief 获取文件大小
        static int64_t GetFileSize(const std::string &path);

        static std::string GetFileExtension(const std::string &path);
        //@brief 获取文件名
        static std::string GetFileName(const std::string &path);

        //@brief 获取父目录
        static std::string GetParentDirectory(const std::string &path);
        //@brief 获取绝对路径
        static std::string GetAbsolutePath(const std::string &path);
        //@brief 获取当前目录
        static std::string GetCurrentDirectory();
        //@brief 设置当前目录
        static void SetCurrentDirectory(const std::string &path);
        //@brief 路径拼接
        static std::string Join(std::initializer_list<std::string> paths);

        //@brief写入文件
        static bool WriteFile(const std::filesystem::path& path, const std::string &data);
        static bool WriteFile(std::filesystem::path & path, const char *content, size_t size);

        //@brief 读取文件
        static bool ReadFile(const std::string &path, std::string &buffer);

    };
}