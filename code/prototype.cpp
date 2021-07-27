// Type your code here, or load an example.
#include <cassert>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <ctype.h>

#include <stdlib.h>
#include <unistd.h> //sysconf

#include <iostream>
#include <vector>

#include <string_view>

template <typename T>
constexpr auto type_name() noexcept
{
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name<";
    suffix = ">(void) noexcept";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

void hexdump(void* pAddressIn, long lSize)
{
    char szBuf[100];
    long lIndent = 1;
    long lOutLen, lIndex, lIndex2, lOutLen2;
    long lRelPos;
    struct {
        char* pData;
        unsigned long lSize;
    } buf;
    unsigned char *pTmp, ucTmp;
    unsigned char* pAddress = (unsigned char*)pAddressIn;

    buf.pData = (char*)pAddress;
    buf.lSize = lSize;
    printf("dumping %li bytes starting at %p\n           0 1 2 3  4 5 6 7  8 9 a b  c d e f  0123456789abcdef\n", lSize, pAddressIn);
    while (buf.lSize > 0) {
        pTmp = (unsigned char*)buf.pData;
        lOutLen = (int)buf.lSize;
        if (lOutLen > 16)
            lOutLen = 16;

        // create a 64-character formatted output line:
        sprintf(szBuf, "%08lX >                            "
                       "                      "
                       "    ",
            pTmp - pAddress);
        lOutLen2 = lOutLen;

        for (lIndex = 9 + lIndent, lIndex2 = 53 - 7 + lIndent, lRelPos = 0;
             lOutLen2;
             lOutLen2--, lIndex += 2, lIndex2++) {
            ucTmp = *pTmp++;

            sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
            if (!isprint(ucTmp))
                ucTmp = '#'; // nonprintable char
            szBuf[lIndex2] = ucTmp;

            if (!(++lRelPos & 3)) // extra blank after 4 bytes
            {
                lIndex++;
                szBuf[lIndex + 2] = ' ';
            }
        }

        if (!(lRelPos & 3))
            lIndex--;

        szBuf[lIndex] = '<';
        szBuf[lIndex + 1] = ' ';

        printf("%s\n", szBuf);

        buf.pData += lOutLen;
        buf.lSize -= lOutLen;
    }
}

struct fastStack {
    void* stackBase; // base of the stack
    void* stackTop; // may not be needed
    void* stack; // current item
    int fd; // dev null file descriptor
    size_t length;

    void createStack()
    {
        int fd;

        fd = open("/dev/zero", O_RDONLY);
        if (fd == -1) {

            fprintf(stderr, "failed to open /dev/zero");
            exit(-1);
        }

        /* offset for mmap() must be page aligned */

        length = 2147483648;
        fprintf(stderr, "length=%i prot=%i flags=%i fd=%i offset=%i pagesize=%i\n",
            length,
            PROT_READ,
            MAP_PRIVATE,
            fd,
            0,
            sysconf(_SC_PAGE_SIZE));

        stackBase = (char*)mmap(nullptr,
            length,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE,
            fd,
            0);
        fprintf(stderr, "got map %p =? %p\n", stackBase, MAP_FAILED);
        if (stackBase == MAP_FAILED) {
            fprintf(stderr, "failed to map /dev/zero");
            exit(-1);
        }
        stackTop = stackBase;
    }

    void dump()
    {
        hexdump(stackBase, (char*)stackTop - (char*)stackBase);
    }

    bool tetsStack()
    {
        for (char c = 'A'; c <= 'z'; c++) {
            push(c);
            getPaddingRequierments<int>();
        }
        dump();
        while(pop<char>() != 'A');
        return true;
    }

    void destroyStack()
    {
        munmap(stackBase, length);
        close(fd);
    }
    template <typename T>
    T* push(T obj)
    {
        //fprintf(stderr, "Pushing %s %s %i\n", std::string(type_name<decltype(obj)>()).c_str(), std::string(obj).c_str(), ((char*)stackTop - (char*)stackBase));
        std::cerr << "pushing " << type_name<decltype(obj)>() << " " << obj << " 0x" << std::hex << ((char*)stackTop - (char*)stackBase) << std::dec << " " << sizeof(T) << "b" << std::endl;
        *(T*)stackTop = obj;
        stackTop = (char*)stackTop + sizeof(T);
        return (T*)stackTop;
    }
    template <typename T>
    T pop()
    {
        T tmp;
        stackTop = (char*)stackTop - sizeof(T);
        std::cerr << "popping " << type_name<decltype(tmp)>() << " " << *(T*)stackTop << " 0x" << std::hex << ((char*)stackTop - (char*)stackBase) << std::dec << " " << sizeof(T) << "b" << std::endl;

        return *(T*)stackTop;
    }

    template <typename T>
    size_t getPaddingRequierments(){
        T tmp;
        std::cerr << "needs " << (std::alignment_of<T>::value - (long unsigned int)stackTop) % std::alignment_of<T>::value << "b of alignment for " << type_name<decltype(tmp)>() << std::endl;
        return (std::alignment_of<T>::value - (long unsigned int)stackTop) % std::alignment_of<T>::value;
    }

    void addAlingnment(char sizeMinusOne)//0 indexing allows for 256 bytes of padding
    {
        stackTop = (char*)stackTop + sizeMinusOne + sizeof(char);
        char* top = (char*)stackTop;
        *(char*)stackTop = sizeMinusOne;
    }
    void removeAlignment()
    {
        stackTop = (char*)stackTop - *(char*)stackTop - sizeof(char);
    }
    fastStack() { createStack(); }
    ~fastStack() { destroyStack(); }
};

// ops +-*/%&|^

void applyop(void* stack) { int* op = (int*)stack; }

int main()
{
    try {
        fastStack stack;
        std::cout << "setup" << std::endl;
        stack.push<int>('c');
        stack.dump();
        stack.getPaddingRequierments<int>();
        std::cout << "puched" << std::endl;
        std::cout << stack.pop<int>() << std::endl;
        std::cout << "poped" << std::endl;
        stack.tetsStack();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}
