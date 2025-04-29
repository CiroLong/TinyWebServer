#include "buffer.h"

Buffer::Buffer(int initBufferSize) : buffer_(initBufferSize), readPos_(0), writePos_(0) {}

// 可写的数量：buffer大小 - 写下标
size_t Buffer::WritableBytes() const
{
    return buffer_.size() - writePos_;
}

// 可读的数量：写下标 - 读下标
size_t Buffer::ReadableBytes() const
{
    return writePos_ - readPos_;
}

// 可预留空间：已经读过的就没用了，等于读下标
size_t Buffer::PrependableBytes() const
{
    return readPos_;
}

const char *Buffer::Peek() const
{
    return BeginPtr_() + readPos_;
}

void Buffer::EnsureWriteable(size_t len)
{
    if (len > WritableBytes())
        MakeSpace_(len);
    assert(len <= WritableBytes());
}

// 移动写下标，在Append中使用
void Buffer::HasWritten(size_t len)
{
    writePos_ += len;
}

// 读取len长度，移动读下标
void Buffer::Retrieve(size_t len)
{
    assert(len <= ReadableBytes());
    readPos_ += len;
}

// 读到end为止
void Buffer::RetrieveUntil(const char *end)
{
    assert(Peek() <= end);
    Retrieve(end - Peek());
}

// 取出所有数据，buffer归零，读写下标归零,在别的函数中会用到
void Buffer::RetrieveAll()
{
    bzero(&buffer_[0], buffer_.size());
    readPos_ = writePos_ = 0;
}

// 取出All -> str
std::string Buffer::RetrieveAllToStr()
{
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char *Buffer::BeginWriteConst() const
{
    return BeginPtr_() + writePos_;
}

char *Buffer::BeginWrite()
{
    return BeginPtr_() + writePos_;
}

void Buffer::Append(const char *str, size_t len)
{
    assert(str != nullptr);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const std::string &str)
{
    Append(str.data(), str.length());
}

void Buffer::Append(const void *data, size_t len)
{
    assert(data);
    Append(static_cast<const char *>(data), len);
}

void Buffer::Append(const Buffer &buff)
{
    Append(buff.Peek(), buff.ReadableBytes());
}

// 将fd的内容读到缓冲区，即writable的位置
// 在非阻塞网络编程中，如何设计并使用缓冲区？
// 一方面我们希望减少系统调用，一次读的数据越多越划算，那么似乎应该准备一个大的缓冲区。
// 另一方面，我们系统减少内存占用。如果有 10k 个连接，每个连接一建立就分配 64k 的读缓冲的话，将占用 640M 内存，而大多数时候这些缓冲区的使用率很低。
// muduo 用 readv 结合栈上空间巧妙地解决了这个问题。

// 在栈上准备一个 65536 字节的 stackbuf，然后利用 readv() 来读取数据，
// iovec 有两块，第一块指向 muduo Buffer 中的 writable 字节，另一块指向栈上的 stackbuf。
// 这样如果读入的数据不多，那么全部都读到 Buffer 中去了；
// 如果长度超过 Buffer 的 writable 字节数，就会读到栈上的 stackbuf 里，
// 然后程序再把 stackbuf 里的数据 append 到 Buffer 中。

// 这么做利用了临时栈上空间，避免开巨大 Buffer 造成的内存浪费，也避免反复调用 read() 的系统开销（通常一次 readv() 系统调用就能读完全部数据）。
ssize_t Buffer::ReadFd(int fd, int *Errno)
{
    char buff[65535];
    struct iovec iov[2]; /// ???
    size_t writeable = WritableBytes();
    // 分散读， 保证数据全部读完
    iov[0].iov_base = BeginWrite(); // ?这是干什么用的
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if (len < 0)
    {
        *Errno = errno;
    }
    else if (static_cast<size_t>(len) <= writeable)
    {
        writePos_ += len;
    }
    else
    {
        writePos_ = buffer_.size();
        Append(buff, static_cast<size_t>(len - writeable)); // 剩余的长度
    }

    return len;
}

// 将buffer中可读的区域写入fd中
ssize_t Buffer::WriteFd(int fd, int *Errno)
{
    ssize_t len = write(fd, Peek(), ReadableBytes());
    if (len < 0)
    {
        *Errno = errno;
        return len;
    }
    Retrieve(len);
    return len;
}

char *Buffer::BeginPtr_()
{
    return &buffer_[0];
}

const char *Buffer::BeginPtr_() const
{
    return &buffer_[0];
}

void Buffer::MakeSpace_(size_t len)
{
    // 第一段 + 第三段 < len 在后面拓展
    if (WritableBytes() + PrependableBytes() < len)
    {
        buffer_.resize(writePos_ + len + 1);
    }
    else
    // 前移
    {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readable;
        assert(readable == ReadableBytes());
    }
}
