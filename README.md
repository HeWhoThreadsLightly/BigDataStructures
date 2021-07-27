# BigDataStructures

Goal is to implement common datatypes in a big automatically allocating structure inspired by EVector: An Efficient Vector Implementation – Using Virtual Memory for Improving Memory 

Prototype version implementing a stack that supports mixing data types like a program stack.
Note that the stack grows up and padding must be added and removed manually using the getPaddingRequierments<T>() addAlingnment(char) and removeAlignment() functions in a maximum of 256 bytes of padding at a time.
https://godbolt.org/z/Mv8Pjo7qb
