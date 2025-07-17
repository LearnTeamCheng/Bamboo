import os

def collect_sources(root_dir, extensions=('.cpp', '.c', '.h', '.hpp')):
    files = []
    for dirpath, _, filenames in os.walk(root_dir):
        for f in filenames:
            if f.endswith(extensions):
                rel_path = os.path.relpath(os.path.join(dirpath, f), root_dir)
                files.append(rel_path.replace('\\', '/'))
    return files

def generate_cmake(module_name, source_list):
    cmake = []
    cmake.append(f'project({module_name})\n')
    cmake.append(f'add_library({module_name} STATIC')
    for src in source_list:
        cmake.append(f'    {src}')
    cmake.append(')')
    cmake.append(f'target_include_directories({module_name} PUBLIC ${{CMAKE_CURRENT_SOURCE_DIR}})')
    cmake.append(f'target_link_libraries({module_name} PUBLIC spdlog glfw entt glm)')
    return ' \n'.join(cmake)

if __name__ == '__main__':
    #当前目录
    current_dir = os.path.dirname(os.path.abspath(__file__))
    #上级目录
    parent_dir = os.path.dirname(current_dir)
    module_dir = 'Bamboo'
    path = os.path.join(parent_dir, module_dir)
    sources = collect_sources(path)
    cmake_content = generate_cmake('Bamboo', sources)

    with open(os.path.join(parent_dir,module_dir, 'CMakeLists.txt'), 'w') as f:
        f.write(cmake_content)

    print(f'✅ Generated CMakeLists.txt for {module_dir}')