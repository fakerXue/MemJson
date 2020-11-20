#include "DatX.h"

using namespace x2lib;

DatX::DatX()
{
    Build(nullptr, 0);
}

DatX::DatX(void* pMem, int nLen)
{
    Build(pMem, nLen);
}

DatX::DatX(const char* szKey, const char* szFmt, ...)
{
    va_list body;
    va_start(body, szFmt);
    _Put(szKey, szFmt, body);
    va_end(body);
}

DatX::DatX(const char* szKey, int nBin, void* pBin)
{
    __Put(szKey, nBin, pBin);
}

DatX::~DatX()
{
    if (__parent__ == nullptr)
    { // 对根节点进行释放
        free((void*)__mem__);
    }
}

unsigned int DatX::Mem() const
{
    return __mem__;
}

unsigned int DatX::Len() const
{
    return __len__;
};

unsigned int DatX::Cnt() const
{
    //if (__cnt__ == 0)
    //{
    //	XTY x_temp(__mem__);
    //	for (unsigned int i = 0;; ++i)
    //	{
    //		if (((unsigned int)x_temp.p - __mem__) + x_temp.n >= __len__)
    //			return i; // 已经到最后一个键
    //		int xvlen = strlen(x_temp.k) + 1 + sizeof(x_temp.n) + x_temp.n;
    //		x_temp.Read(xvlen + ((unsigned int)x_temp.k), i);
    //	}
    //}
    return __cnt__;
}

DatX& DatX::Put(const char* szKey, int iVal)
{
    return Put(szKey, "%d", iVal);
}

DatX& DatX::Put(const char* szKey, double dVal)
{
    return Put(szKey, "%.6f", dVal);
}

DatX& DatX::Put(const char* szKey, char* sVal)
{
    return Put(szKey, "%s", sVal);
}

DatX& DatX::Put(const char* szKey, const char* szFmt, ...)
{
    va_list body;
    va_start(body, szFmt);
    _Put(szKey, szFmt, body);
    va_end(body);
    return *this;
}

DatX& DatX::Put(const char* szKey, unsigned int nBin, void* pBin)
{
    return __Put(szKey, nBin, pBin);
}

DatX& DatX::__Put(const char* szKey, unsigned int nBin, void* pBin)
{
#if 1 // 暂时废弃__type__，使之支持混杂数组，后续看是否有必要开启
    __type__ = '?';
#else
    if (szKey == nullptr || szKey[0] == 0)
    { // Add
        if (__type__ != 'V' && __type__ != '?')
        {
            return *(DatX*)nullptr; // 强制程序崩溃
        }
        __type__ = 'V';
    }
    else
    { // Put
        if (__type__ != 'K' && __type__ != '?')
        {
            return *(DatX*)nullptr; // 强制程序崩溃
        }
        __type__ = 'K';
    }
#endif

    int klen = strlen(szKey) + 1;
    unsigned int newlen = klen + sizeof(XTY::n) + nBin;
    unsigned int p_mem = 0;

    XTY x = this->Get(szKey);
    if (!x.IsNull())
    {
        p_mem = x._p_;
        move_memory(&p_mem, x._n_, newlen);
        // 若p_mem被move_memory更改，则x失效，但由于后续没再使用x，因此忽略更新
    }
    else
    {
        if (__mem__ == 0/* || __len__ == 0*/) newlen += sizeof(DatX::__len__); // 根节点首次申请内存
        p_mem = __mem__ + __len__;
        move_memory(&p_mem, 0, newlen);
        //if (p_mem == 0)
        //{
        //    p_mem = __mem__ + sizeof(DatX::__len__);
        //    // __len__ += sizeof(DatX::__len__);
        //}
    }

    memcpy((void*)p_mem, szKey, klen);
    *(unsigned int*)(p_mem + klen) = nBin;
    memcpy((void*)(p_mem + klen + sizeof(XTY::n)), pBin, nBin);
    ++__cnt__;

    return *this;
}

DatX& DatX::_Put(const char* szKey, const char* szFmt, va_list body)
{
    char* pData = (char*)malloc(DatX::COB);
    int nData = vsnprintf(pData, DatX::COB, (char*)szFmt, body) + 1;
    if (nData > DatX::COB)
    {
        pData = (char*)realloc(pData, nData);
        vsnprintf(pData, nData, (char*)szFmt, body);
    }
    pData[nData] = 0;
    __Put(szKey, nData, pData);
    free(pData);

    return *this;
}

DatX& DatX::Put(const char* szKey, DatX& dx)
{
    return __Put(szKey, dx.Len(), (void*)dx.Mem());
}

DatX& DatX::Add(int iVal)
{
    return Add("%d", iVal);
}

DatX& DatX::Add(double dVal)
{
    return Add("%.6f", dVal);
}

DatX& DatX::Add(char* sVal)
{
    return Add("%s", sVal);
}

DatX& DatX::Add(const char* szFmt, ...)
{
    va_list body;
    va_start(body, szFmt);
    DatX& dx = _Put("", szFmt, body);
    va_end(body);
    return dx;
}

DatX& DatX::Add(unsigned int nBin, void* pBin)
{
    return __Put("", nBin, pBin);
}

DatX& DatX::Add(DatX& dx)
{
    return Add(dx.Len(), (void*)dx.Mem());
}

void DatX::move_memory(unsigned int* p_mem, unsigned int len, unsigned int xlen)
{
    int diff = xlen - len;

    DatX* p_root = this;
    while (p_root->__parent__)
    {
        p_root->__len__ += diff;
        *(unsigned int*)p_root->__mem__ = p_root->__len__; // 更新当前节点__len__
        *(unsigned int*)(p_root->__mem__ - sizeof(XTY::n)) = p_root->__len__; // 更新当前节点在所属XTY的n
        p_root->__cap__ = p_root->__len__; // 非根节点保持__cap__==__len__
        p_root = p_root->__parent__;
    }

    // 搬运数据
    if (diff < 0)
    { // 数据收缩，前移mem+len处的数据
        memmove((void*)(*p_mem), (void*)((*p_mem) + len), p_root->__len__ - ((*p_mem) + len - p_root->__mem__));
        p_root->__len__ += diff;
        *(unsigned int*)p_root->__mem__ = p_root->__len__;
        int _collect_bytes = p_root->__cap__ - (p_root->__len__ + DatX::COB * 4);
        if (_collect_bytes > 0)
        { // 垃圾回收
            p_root->__cap__ -= _collect_bytes;
            realloc((void*)__mem__, __cap__);
        }
    }
    else if (diff > 0)
    { // 数据扩充，后移mem处的数据
        unsigned int newlen = p_root->__len__ + diff;
        unsigned int __mem__ofs_ = (*p_mem) - p_root->__mem__; // 计算偏移，因为realloc后__mem__可能会变
        if (newlen > p_root->__cap__)
        {
            p_root->__cap__ += (diff > DatX::COB ? diff : DatX::COB);
            p_root->__mem__ = (unsigned int)realloc((void*)p_root->__mem__, p_root->__cap__);
        }
        *(unsigned int*)p_root->__mem__ = newlen; // 只能放在realloc后，使得可以处理未初始化的根节点
        if ((*p_mem) == 0) { (*p_mem) = p_root->__mem__ + sizeof(DatX::__len__); }
        else { (*p_mem) = p_root->__mem__ + __mem__ofs_; }  // 更新并回传给调用者

        if (len != 0) { memmove((void*)((*p_mem) + xlen), (void*)(*p_mem), p_root->__len__ - __mem__ofs_); }
        p_root->__len__ = newlen;
    }
}

bool DatX::Del(const char *szKey)
{
    XTY x = this->Get(szKey);
    return Del(x.i);
}

bool DatX::Del(int iKey)
{
    XTY x = this->Get(iKey);
    if (!x.IsNull())
    {
        move_memory(&x._p_, x._n_, 0); // 一旦x._p_被move_memory更改，则x失效
        --__cnt__;
        return true;
    }
    return false;
}

DatX DatX::operator[](const char* szKey)
{
    DatX dx;
    dx.__parent__ = this;

    XTY x = Get(szKey);
    if (!x.IsNull())
    {
        if (DatX::IsValid((void*)x.v, x.n, &dx.__cnt__))
        {
            dx.__mem__ = (unsigned int)x.v; // 与()的不同之处
            dx.__len__ = x.n;
            dx.__cap__ = x.n;
            dx.__type__ = 'V';
        }
    }
    else
    {
        unsigned int len = sizeof(DatX::__len__);
        __Put(szKey, sizeof(len), &len); // 新增一个值为sizeof(DatX::__len__)的键值对
        dx.__mem__ = (unsigned int)Get(szKey).v;
        dx.__cap__ = dx.__len__ = sizeof(DatX::__len__);
    }

    return dx;
}

DatX DatX::operator[](int iKey)
{
    return (*this)[Get(iKey).k];
}
#if 0
DatX DatX::operator()(const char* szKey)
{ // faker@2020-11-14 10:34:13 TODO 以下代码有误，需要纠正
    DatX dx;
    dx.__parent__ = this;

    XTY x = Get(szKey);
    if (!x.IsNull())
    {
        dx.Build(x.v, x.n);
        return dx;
    }
    else
    {
        unsigned int len = sizeof(DatX::__len__);
        __Put(szKey, (unsigned char*)&len, sizeof(len)); // 新增一个值为sizeof(DatX::__len__)的键值对
        dx.__mem__ = (unsigned int)Get(szKey).v;
        dx.__cap__ = dx.__len__ = sizeof(DatX::__len__);
    }

    return dx;
}

DatX DatX::operator()(int iKey)
{
    return (*this)[Get(iKey).k];
}
#endif

DatX::XTY DatX::Get(unsigned int iKey)
{
    if (__mem__ != 0 && iKey >= 0 && iKey < __cnt__)
    {
        XTY x_temp(__mem__ + sizeof(DatX::__len__));
        for (unsigned int i = 0;;)
        {
            if (i == iKey) return x_temp;
            if (++i >= __cnt__) break;
            x_temp.Read((unsigned int)x_temp._p_ + x_temp._n_, i);
        }
    }

    return XTY();
}

DatX::XTY DatX::Get(const char* szKey)
{
    if (szKey && szKey[0] && __mem__ != 0 && __cnt__ != 0)
    {
        XTY x_temp(__mem__ + sizeof(DatX::__len__));
        for (unsigned int i = 0;;)
        {
            if (x_temp.IsNull())
                break;
            if (strcmp(x_temp.k, szKey) == 0)
                return x_temp;
            if (++i >= __cnt__)
                break;
            x_temp.Read((unsigned int)x_temp._p_ + x_temp._n_, i);
        }
    }

    return XTY();
}

int DatX::LastError()
{
    return __err__;
}

bool DatX::Build(void* pMem, int nLen)
{
    if (!DatX::IsValid(pMem, nLen, &__cnt__))
    {
        memset(this, 0, sizeof(DatX));
        __type__ = '?';
        return false;
    }
    move_memory(&__mem__, __len__, nLen); // 待验证！！！！
    memcpy((void*)__mem__, pMem, nLen);
    __len__ = nLen;
    if (((char*)(__mem__ + sizeof(DatX::__len__)))[0] == 0) __type__ = 'V';
    else __type__ = 'K';
    return true;
}

bool DatX::IsValid()
{
    return DatX::IsValid((void*)__mem__, __len__, &__cnt__);
}

bool DatX::IsValid(void* pMem, int nLen, unsigned int* pnCnt)
{
    if (pnCnt != nullptr) *pnCnt = 0;
    if (!pMem || nLen == 0 || (*(unsigned int*)pMem) != nLen)
        return false;

    XTY x_temp((unsigned int)pMem + sizeof(DatX::__len__));
    unsigned int i = 0;
    int sumlen = sizeof(DatX::__len__);
    do
    {
        if (x_temp.IsNull())
            return false;
        sumlen += x_temp._n_;
        if (sumlen > nLen) return false;

        ++i;
        if (sumlen == nLen) break;

        x_temp.Read((unsigned int)pMem + sumlen, i);
    } while (true);
    if (pnCnt != nullptr) *pnCnt = i;
    return true;
}

