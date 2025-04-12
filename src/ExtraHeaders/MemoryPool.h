#pragma once

#include <CriticalSection.h>
#include <Utilities/Debug.h>

//--------------------------------------------------------------------
// Fundamental type definitions
//--------------------------------------------------------------------
typedef float							Real;							// 4 bytes 
static_assert(sizeof(Real) == 0x4, "InvalidSize");
typedef int								Int;							// 4 bytes 
static_assert(sizeof(Int) == 0x4, "InvalidSize");
typedef unsigned int			UnsignedInt;	  	// 4 bytes 
static_assert(sizeof(UnsignedInt) == 0x4, "InvalidSize");
typedef unsigned short		UnsignedShort;		// 2 bytes 
static_assert(sizeof(UnsignedShort) == 0x2, "InvalidSize");
typedef short							Short;					  // 2 bytes 
static_assert(sizeof(Short) == 0x2, "InvalidSize");
typedef unsigned char			UnsignedByte;			// 1 byte		USED TO BE "Byte"
static_assert(sizeof(UnsignedByte) == 0x1, "InvalidSize");
typedef char							Byte;							// 1 byte		USED TO BE "SignedByte"
static_assert(sizeof(Byte) == 0x1, "InvalidSize");
typedef char							Char;							// 1 byte of text
static_assert(sizeof(Char) == 0x1, "InvalidSize");
typedef bool							Bool;							// 
static_assert(sizeof(Bool) == 0x1, "InvalidSize");
// note, the types below should use "long long", but MSVC doesn't support it yet
typedef __int64						Int64;							// 8 bytes 
static_assert(sizeof(Int64) == 0x8, "InvalidSize");
typedef unsigned __int64	UnsignedInt64;	  	// 8 bytes 
static_assert(sizeof(UnsignedInt64) == 0x8, "InvalidSize");

enum
{
	MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS = 8	///< The max number of subpools allowed in a DynamicMemoryAllocator
};

enum ErrorCode
{
	ERROR_BASE = 0xdead0001,								// a nice, distinctive value

	ERROR_BUG = (ERROR_BASE + 0x0000),		///< should not be possible under normal operation
	ERROR_OUT_OF_MEMORY = (ERROR_BASE + 0x0001),		///< unable to allocate memory.
	ERROR_BAD_ARG = (ERROR_BASE + 0x0002),		///< generic "bad argument".
	ERROR_INVALID_FILE_VERSION = (ERROR_BASE + 0x0003),		///< Unrecognized file version.
	ERROR_CORRUPT_FILE_FORMAT = (ERROR_BASE + 0x0004),		///< Invalid file format.
	ERROR_BAD_INI = (ERROR_BASE + 0x0005),		///< Bad INI data.
	ERROR_INVALID_D3D = (ERROR_BASE + 0x0006),    ///< Error initing D3D 

	ERROR_LAST
};

struct PoolInitRec
{
	const char* poolName;					///< name of the pool; by convention, "dmaPool_XXX" where XXX is allocationSize
	Int allocationSize;						///< size, in bytes, of the pool.
	Int initialAllocationCount;		///< initial number of blocks to allocate.
	Int overflowAllocationCount;	///< when the pool runs out of space, allocate more blocks in this increment
};

struct PoolSizeRec
{
	const char* name;
	Int initial;
	Int overflow;
};

class CriticalSection
{
	CRITICAL_SECTION m_windowsCriticalSection;

public:

	CriticalSection()
	{
		InitializeCriticalSection(&m_windowsCriticalSection);
	}

	virtual ~CriticalSection()
	{
		DeleteCriticalSection(&m_windowsCriticalSection);
	}

public:	// Use these when entering/exiting a critical section.

	void enter(void)
	{
		EnterCriticalSection(&m_windowsCriticalSection);
	}

	void exit(void)
	{
		LeaveCriticalSection(&m_windowsCriticalSection);
	}
};

class ScopedCriticalSection
{
private:
	CriticalSection* m_cs;

public:
	ScopedCriticalSection(CriticalSection* cs) : m_cs(cs)
	{
		if (m_cs)
			m_cs->enter();
	}

	virtual ~ScopedCriticalSection()
	{
		if (m_cs)
			m_cs->exit();
	}
};


class MemoryPoolFactory;
class MemoryPoolBlob;
class MemoryPoolSingleBlock
{
private:

	MemoryPoolBlob* m_owningBlob;			///< will be NULL if the single block was allocated via sysAllocate()
	MemoryPoolSingleBlock* m_nextBlock;				///< if m_owningBlob is nonnull, this points to next free (unallocated) block in the blob; if m_owningBlob is null, this points to the next used (allocated) raw block in the pool.

private:

	void* getUserDataNoDbg();

public:

	static Int calcRawBlockSize(Int logicalSize);

	static MemoryPoolSingleBlock* rawAllocateSingleBlock(MemoryPoolSingleBlock** pRawListHead, Int logicalSize, MemoryPoolFactory* owningFactory);

	void removeBlockFromList(MemoryPoolSingleBlock** pHead);
	void initBlock(Int logicalSize, MemoryPoolBlob* owningBlob, MemoryPoolFactory* owningFactory);
	void* getUserData();

	static MemoryPoolSingleBlock* recoverBlockFromUserData(void* pUserData);

	MemoryPoolBlob* getOwningBlob();
	MemoryPoolSingleBlock* getNextFreeBlock();
	void setNextFreeBlock(MemoryPoolSingleBlock* b);
	MemoryPoolSingleBlock* getNextRawBlock();
	void setNextRawBlock(MemoryPoolSingleBlock* b);

};

class MemoryPool;
class MemoryPoolBlob
{
private:
	MemoryPool* m_owningPool;				///< the pool that owns this blob
	MemoryPoolBlob* m_nextBlob;					///< next blob in this pool
	MemoryPoolBlob* m_prevBlob;					///< prev blob in this pool
	MemoryPoolSingleBlock* m_firstFreeBlock;		///< ptr to first available block in this blob
	Int											m_usedBlocksInBlob;		///< total allocated blocks in this blob
	Int											m_totalBlocksInBlob;	///< total blocks in this blob (allocated + available)
	char* m_blockData;					///< ptr to the blocks (really a MemoryPoolSingleBlock*)

public:

	MemoryPoolBlob();
	~MemoryPoolBlob();

	void initBlob(MemoryPool* owningPool, Int allocationCount);
	void addBlobToList(MemoryPoolBlob** ppHead, MemoryPoolBlob** ppTail);
	void removeBlobFromList(MemoryPoolBlob** ppHead, MemoryPoolBlob** ppTail);

	MemoryPoolBlob* getNextInList();
	Bool hasAnyFreeBlocks();

	MemoryPoolSingleBlock* allocateSingleBlock();

	void freeSingleBlock(MemoryPoolSingleBlock* block);

	MemoryPool* getOwningPool();
	Int getFreeBlockCount();
	Int getUsedBlockCount();
	Int getTotalBlockCount();

};

class MemoryPoolFactory;
class MemoryPool
{
private:

	MemoryPoolFactory* m_factory;									///< the factory that created us
	MemoryPool* m_nextPoolInFactory;				///< linked list node, managed by factory
	const char* m_poolName;								///< name of this pool. (literal string; must not be freed)
	Int								m_allocationSize;						///< size of the blocks allocated by this pool, in bytes
	Int								m_initialAllocationCount;		///< number of blocks to be allocated in initial blob
	Int								m_overflowAllocationCount;	///< number of blocks to be allocated in any subsequent blob(s)
	Int								m_usedBlocksInPool;					///< total number of blocks in use in the pool.
	Int								m_totalBlocksInPool;				///< total number of blocks in all blobs of this pool (used or not).
	Int								m_peakUsedBlocksInPool;			///< high-water mark of m_usedBlocksInPool
	MemoryPoolBlob* m_firstBlob;								///< head of linked list: first blob for this pool.
	MemoryPoolBlob* m_lastBlob;								///< tail of linked list: last blob for this pool. (needed for efficiency)
	MemoryPoolBlob* m_firstBlobWithFreeBlocks;	///< first blob in this pool that has at least one unallocated block.

private:
	/// create a new blob with the given number of blocks.
	MemoryPoolBlob* createBlob(Int allocationCount);
	/// destroy a blob.
	Int freeBlob(MemoryPoolBlob* blob);

public:

	// 'public' funcs that are really only for use by MemoryPoolFactory
	MemoryPool* getNextPoolInList();	///< return next pool in linked list
	
	void addToList(MemoryPool** pHead);		///< add this pool to head of the linked list
	void removeFromList(MemoryPool** pHead);	///< remove this pool from the linked list

public:

	MemoryPool();

	/// initialize the given memory pool.
	void init(MemoryPoolFactory* factory, const char* poolName, Int allocationSize, Int initialAllocationCount, Int overflowAllocationCount);

	~MemoryPool();

	/// allocate a block from this pool. (don't call directly; use allocateBlock() macro)
	void* allocateBlockImplementation();

	/// same as allocateBlockImplementation, but memory returned is not zeroed
	void* allocateBlockDoNotZeroImplementation();

	/// free the block. it is OK to pass null.
	void freeBlock(void* pMem);

	/// return the factory that created (and thus owns) this pool.
	MemoryPoolFactory* getOwningFactory();

	/// return the name of this pool. the result is a literal string and must not be freed.
	const char* getPoolName();

	/// return the block allocation size of this pool.
	Int getAllocationSize();

	/// return the number of free (available) blocks in this pool.
	Int getFreeBlockCount();

	/// return the number of blocks in use in this pool.
	Int getUsedBlockCount();

	/// return the total number of blocks in this pool. [ == getFreeBlockCount() + getUsedBlockCount() ]
	Int getTotalBlockCount();

	/// return the high-water mark for getUsedBlockCount()
	Int getPeakBlockCount();

	/// return the initial allocation count for this pool
	Int getInitialBlockCount();

	Int countBlobsInPool();

	/// if this pool has any empty blobs, return them to the system.
	Int releaseEmpties();

	/// destroy all blocks and blobs in this pool.
	void reset();
};

class MemoryPoolObject
{
protected:

	/** ensure that all destructors are virtual */
	virtual ~MemoryPoolObject() { }

protected:
	inline void* operator new(size_t s) { Debug::FatalError(("This should be impossible")); return 0; }
	inline void operator delete(void* p) { Debug::FatalError(("This should be impossible")); }

protected:

	virtual MemoryPool* getObjectMemoryPool() = 0;

public:

	void deleteInstance();
};

template<typename T>
concept IsMemoryPoolObject = std::is_base_of<MemoryPoolObject, T>::value;

class DynamicMemoryAllocator;
class MemoryPoolFactory
{
private:
	MemoryPool* m_firstPoolInFactory;		///< linked list of pools
	DynamicMemoryAllocator* m_firstDmaInFactory;			///< linked list of dmas

public:

	MemoryPoolFactory();
	void init() { }
	~MemoryPoolFactory();

	/// create a new memory pool with the given settings. if a pool with the given name already exists, return it.
	MemoryPool* createMemoryPool(const PoolInitRec* parms);

	/// overloaded version of createMemoryPool with explicit parms.
	MemoryPool* createMemoryPool(const char* poolName, Int allocationSize, Int initialAllocationCount, Int overflowAllocationCount);

	/// return the pool with the given name. if no such pool exists, return null.
	MemoryPool* findMemoryPool(const char* poolName);

	/// destroy the given pool.
	void destroyMemoryPool(MemoryPool* pMemoryPool);

	/// create a DynamicMemoryAllocator with subpools with the given parms.
	DynamicMemoryAllocator* createDynamicMemoryAllocator(Int numSubPools, const PoolInitRec pParms[]);

	/// destroy the given DynamicMemoryAllocator.
	void destroyDynamicMemoryAllocator(DynamicMemoryAllocator* dma);

	/// destroy the contents of all pools and dmas. (the pools and dma's are not destroyed, just reset)
	void reset();

	void reportAllocation();
};

extern MemoryPoolFactory* TheMemoryPoolFactory;

class DynamicMemoryAllocator
{
private:
	MemoryPoolFactory* m_factory;						///< the factory that created us
	DynamicMemoryAllocator* m_nextDmaInFactory;	///< linked list node, managed by factory
	Int												m_numPools;						///< number of subpools (up to MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS)
	Int												m_usedBlocksInDma;		///< total number of blocks allocated, from subpools and "raw"
	MemoryPool* m_pools[MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS];	///< the subpools
	MemoryPoolSingleBlock* m_rawBlocks;					///< linked list of "raw" blocks allocated directly from system

	/// return the best pool for the given allocSize, or null if none are suitable
	MemoryPool* findPoolForSize(Int allocSize);

public:

	// 'public' funcs that are really only for use by MemoryPoolFactory
	///< return next dma in linked list
	DynamicMemoryAllocator* getNextDmaInList();

	///< add this dma to the list
	void addToList(DynamicMemoryAllocator** pHead);

	///< remove this dma from the list
	void removeFromList(DynamicMemoryAllocator** pHead);

public:

	DynamicMemoryAllocator();

	/// initialize the dma. pass 0/null for numSubPool/parms to get some reasonable default subpools.
	void init(MemoryPoolFactory* factory, Int numSubPools, const PoolInitRec pParms[]);

	~DynamicMemoryAllocator();

	/// allocate bytes from this pool. (don't call directly; use allocateBytes() macro)
	void* allocateBytesImplementation(Int numBytes);

	/// like allocateBytesImplementation, but zeroes the memory before returning
	void* allocateBytesDoNotZeroImplementation(Int numBytes);


	/// free the bytes. (assumes allocated by this dma.)
	void freeBytes(void* pMem);

	/**
		return the actual number of bytes that would be allocated
		if you tried to allocate the given size. (It will generally be slightly
		larger than you request.) This lets you use extra space if you're gonna get it anyway...
		The idea is that you will call this before doing a memory allocation, to see if
		you got any extra "bonus" space.
	*/
	Int getActualAllocationSize(Int numBytes);

	/// destroy all allocations performed by this DMA.
	void reset();
	Int getDmaMemoryPoolCount() const;
	MemoryPool* getNthDmaMemoryPool(Int i) const;

};

extern DynamicMemoryAllocator* TheDynamicMemoryAllocator;

extern CriticalSection* TheDmaCriticalSection;
extern CriticalSection* TheMemoryPoolCriticalSection;

struct Mem {
	static void userMemoryManagerGetDmaParms(Int* numSubPools, const PoolInitRec** pParms);
	static void userMemoryManagerInitPools();
	static void preMainInitMemoryManager();
	static void shutdownMemoryManager();
};

#define MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS) \
protected: \
	virtual ~ARGCLASS(); \
public: \
	enum ARGCLASS##MagicEnum { ARGCLASS##_GLUE_NOT_IMPLEMENTED = 0 }; \
public: \
	inline void *operator new(size_t s, ARGCLASS##MagicEnum e) \
	{ \
		return ARGCLASS::getClassMemoryPool()->allocateBlockImplementation(); \
	} \
public: \
	/* \
		Note that this delete operator can't be called directly; it is called \
		only if the analogous new operator is called, AND the constructor \
		throws an exception... \
	*/ \
	inline void operator delete(void *p, ARGCLASS##MagicEnum e) \
	{ \
		ARGCLASS::getClassMemoryPool()->freeBlock(p); \
	} \
protected: \
	/* \
		Make normal new and delete protected, so they can't be called by the outside world. \
		Note that delete is funny, in that it can still be called by the class itself; \
		this is safe but not recommended, for consistency purposes. More problematically, \
		it can be called by another class that has declared itself 'friend' to us. \
		In theory, this shouldn't work, since it may not use the right operator-delete, \
		and thus the wrong memory pool; in practice, it seems the right delete IS called \
		in MSVC -- it seems to make operator delete virtual if the destructor is also virtual. \
		At any rate, this is undocumented behavior as far as I can tell, so we put a big old \
		crash into operator delete telling people to do the right thing and call deleteInstance \
		instead -- it'd be nice if we could catch this at compile time, but catching it at \
		runtime seems to be the best we can do... \
	*/ \
	inline void *operator new(size_t s) \
	{ \
		Debug::FatalError(("This operator new should normally never be called... please use new(char*) instead.")); \
		return 0; \
	} \
	inline void operator delete(void *p) \
	{ \
		Debug::FatalError(("Please call deleteInstance instead of delete.")); \
		ARGCLASS::getClassMemoryPool()->freeBlock(p); \
	} \
private: \
	virtual MemoryPool *getObjectMemoryPool() \
	{ \
		return ARGCLASS::getClassMemoryPool(); \
	} \
public: /* include this line at the end to reset visibility to 'public' */ 

#define GCMP_CREATE(ARGCLASS, ARGPOOLNAME, ARGINITIAL, ARGOVERFLOW) \
private: \
	static MemoryPool *getClassMemoryPool() \
	{ \
		/* \
			Note that this static variable will be initialized exactly once: the first time \
			control flows over this section of code. This allows us to neatly resolve the \
			order-of-execution problem for static variables, ensuring this is not executed \
			prior to the initialization of TheMemoryPoolFactory. \
		*/ \
		static MemoryPool *The##ARGCLASS##Pool = TheMemoryPoolFactory->createMemoryPool(ARGPOOLNAME, sizeof(ARGCLASS), ARGINITIAL, ARGOVERFLOW); \
		return The##ARGCLASS##Pool; \
	} 


#define NEW_INSTANCE_FUNC(ARGCLASS)\
public: static ARGCLASS* createInstance() { return new(ARGCLASS::ARGCLASS##_GLUE_NOT_IMPLEMENTED) ARGCLASS; }

#define MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ARGCLASS, ARGPOOLNAME) \
	MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS) \
	GCMP_CREATE(ARGCLASS, ARGPOOLNAME, -1, -1)\
	NEW_INSTANCE_FUNC(ARGCLASS)
