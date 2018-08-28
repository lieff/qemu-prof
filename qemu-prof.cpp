#include <inttypes.h>
#include <string.h>
#include <algorithm> 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <map>

struct block
{
    block() { addr = 0; executed = 0; }
    uint64_t addr, executed;
    std::string func_name;
    std::vector<std::string> instr;
};

struct function
{
    function() { executed = 0; executed_instr = 0; }
    std::string name;
    uint64_t executed, executed_instr;
};

bool cmp_function (function *i, function *j) { return i->executed_instr > j->executed_instr; }

int main(int argc, char **argv)
{
    std::map<std::string, function> funcs;
    std::map<uint64_t, block> blocks;

    struct noop {
        void operator()(...) const {}
    };
    std::shared_ptr<std::istream> infile;
    if (argc < 2)
        infile.reset(&std::cin, noop());
    else
        infile.reset(new std::ifstream(argv[1]));

    std::string line;
    uint64_t total_instr = 0;
    while (std::getline(*infile, line))
    {
        if ('-' == line[0])
            continue;
        if (0 == strncmp(line.c_str(), "IN: ", 4))
        {
            std::string func_name = line.c_str() + 4;
            std::getline(*infile, line);
            if (line.empty())
            {
                printf("warning: empty in_asm block\n");
                continue;
            }
            uint64_t addr = strtoul(line.c_str(), NULL, 16);

            if (0 == func_name.length())
            {
                char buf[128];
                snprintf(buf, sizeof(buf), "loc_%" PRIx64, addr);
                func_name = buf;
            }
            auto it = funcs.find(func_name);
            if (funcs.end() == it)
            {
                function f;
                f.name = func_name;
                funcs[func_name] = f;
                it = funcs.find(func_name);
            }
            function &func = it->second;

            auto it2 = blocks.find(addr);
            if (blocks.end() == it2)
            {
                block b;
                b.addr = addr;
                b.func_name = func.name;
                blocks[addr] = b;
                it2 = blocks.find(addr);
            }
            block &b = it2->second;
            b.instr.push_back(line);
            while (std::getline(*infile, line))
            {
                if (line.empty())
                    break;
                b.instr.push_back(line);
            }
            //printf("%s 0x%" PRIx64 " size=%d\n", func.name.c_str(), b.addr, (int)b.instr.size());
        } else if (line.size() >= 32 && 0 == strncmp(line.c_str(), "Trace ", 6))
        {
            const char *addr_str = strstr(line.c_str(), ":");
            if (!addr_str)
            {
                addr_str = strstr(line.c_str(), "[");
                if (!addr_str)
                {
                    printf("error: unknown trace format\n");
                    break;
                }
                addr_str++;
            }
            uint64_t addr = strtoul(addr_str + 2, NULL, 16);
            const char *func_name = strstr(line.c_str(), "]") + 1; if (*func_name && *func_name == ' ') func_name++;
            //printf("Trace %s 0x%" PRIx64 "\n", func_name, addr);
            auto it = blocks.find(addr);
            if (it == blocks.end())
            {
                printf("warning: block address 0x%" PRIx64 " not found\n", addr);
                continue;
            }
            block &b = it->second;
            function &func = funcs.find(b.func_name)->second;
            b.executed++;
            func.executed++;
            func.executed_instr += b.instr.size();
            total_instr += b.instr.size();
        } else
        {
            printf("error: QEMU log parsing fail: %s\n", line.c_str());
            //exit(1);
        }
    }
    printf("Total executed instructions: %" PRIu64 "\n", total_instr);
    std::vector<function*> func_sort;
    for (auto &it : funcs)
    {
        function &func = it.second;
        func_sort.push_back(&func);
    }
    std::sort(func_sort.begin(), func_sort.end(), cmp_function);
    for (auto &it : func_sort)
    {
        function &func = *it;
        double percent = (double)func.executed_instr/total_instr*100;
        if (percent < 0.5)
            break;
        printf("%s %" PRIu64 " %.3f%%\n", func.name.c_str(), func.executed_instr, percent);
    }
    return 0;
}
