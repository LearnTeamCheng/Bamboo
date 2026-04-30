#include "FileUtlis.h"
#include <fstream>
#include <iostream>

namespace Bamboo
{
    namespace fs = std::filesystem;

    bool FileUtlis::FileExist(const std::string &path)
    {
        std::error_code ec;
        return fs::exists(path,ec);
    }

    std::vector<fs::path> FileUtlis::GetFiles(const fs::path &dir, bool recursive)
    {
        std::vector<fs::path> result;

        if (recursive)
        {
            for (auto &e : fs::recursive_directory_iterator(dir))
                if (!e.is_directory())
                    result.push_back(e.path());
        }
        else
        {
            for (auto &e : fs::directory_iterator(dir))
                if (!e.is_directory())
                    result.push_back(e.path());
        }

        return result;
    }

    bool FileUtlis::IsDirector(const std::string &path)
    {
        return fs::is_directory(path);
    }

    bool FileUtlis::CreateDirectory(const std::string &path)
    {
        try
        {
            return fs::create_directory(path);
        }
        catch (const std::exception &e)
        {
            return false;
        }
    }

    bool FileUtlis::RemoveDirectory(const std::string &path)
    {
        return fs::remove_all(path) > 0;
    }

    bool FileUtlis::RemoveFile(const std::string &path)
    {
        return fs::remove(path);
    }

    bool FileUtlis::CopyFile(const std::string &src, const std::string &dst)
    {
        try
        {
            return fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        }
        catch (...)
        {
            return false;
        }
    }

    bool FileUtlis::MoveFile(const std::string &src, const std::string &dst)
    {
        try
        {
            fs::rename(src, dst);
            return true;
        }
        catch (...)
        {
            // fallback（跨盘）
            if (!CopyFile(src, dst))
                return false;

            return RemoveFile(src);
        }
    }

    void FileUtlis::RenameFile(const std::string &src, const std::string &dst)
    {
        fs::rename(src, dst);
    }

    int64_t FileUtlis::GetFileSize(const std::string &path)
    {
        try
        {
            return fs::file_size(path);
        }
        catch (const std::exception &e)
        {
            return -1;
        }
    }

    std::string FileUtlis::GetFileExtension(const std::string &path)
    {
        fs::path p(path);
        return p.extension().string();
    }

    std::string FileUtlis::GetFileName(const std::string &path)
    {
        fs::path p(path);
        return p.filename().string();
    }

    std::string FileUtlis::GetParentDirectory(const std::string &path)
    {
        fs::path p(path);
        return p.parent_path().string();
    }

    std::string FileUtlis::GetAbsolutePath(const std::string &path)
    {
        return fs::absolute(path).string();
    }

    std::string FileUtlis::GetCurrentDirectory()
    {
        return fs::current_path().string();
    }

    void FileUtlis::SetCurrentDirectory(const std::string &path)
    {
        fs::current_path(path);
    }

    std::string FileUtlis::Join(std::initializer_list<std::string> paths)
    {
        // 使用 fs::path 自动处理平台分隔符
        fs::path result;
        for (const auto &p : paths)
        {
            if (result.empty())
                result = p;
            else
                result /= p; // 自动添加正确的分隔符(/ 或 \)
        }
        return result.string();
    }

    bool FileUtlis::WriteFile(fs::path & path, const char *content, size_t size)
    {
        namespace fs = fs;

        // 1. 获取父目录路径
        fs::path pathObj(path);
        fs::path pathDir = pathObj.parent_path();

        // 2. 如果父目录不为空且不存在，则创建（修正了逻辑错误）
        if (!pathDir.empty() && !fs::exists(pathDir))
        {
            if (!fs::create_directories(pathDir))
            {
                return false; // 目录创建失败
            }
        }

        // 3. 以二进制模式打开文件
        std::ofstream ofs(path, std::ios::binary);
        if (!ofs.is_open())
        {
            return false; // 文件打开失败
        }

        // 4. 写入指定大小的数据（注意：不是字符串，而是二进制数据）
        ofs.write(content, size);

        // 5. 检查是否写入成功
        bool success = ofs.good();
        ofs.close();

        return success;
    }

    // 可选：添加一个重载版本方便写字符串
    bool FileUtlis::WriteFile(const fs::path &path, const std::string &content)
    {
        try
        {
            auto dir = path.parent_path();
            if (!dir.empty())
                fs::create_directories(dir);

            auto temp = path;
            temp += ".tmp";

            {
                std::ofstream ofs(temp, std::ios::binary);
                if (!ofs)
                    return false;
                ofs.write(content.data(), content.size());
            }

            fs::rename(temp, path);
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "WriteFile error: " << e.what() << std::endl;
            return false;
        }
    }

    bool FileUtlis::ReadFile(const std::string &path, std::string &buffer)
    {
        // 打开文件
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs)
            return false;

        ifs.seekg(0, std::ios::end);
        size_t size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        buffer.resize(size);
        ifs.read(buffer.data(), size);

        return ifs.good() || ifs.eof();
    }

} // namespace Bamboo