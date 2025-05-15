#include "MemoryPool.h"

/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

#define MEM_BOUND_ALIGNMENT 4

MemoryPoolFactory* TheMemoryPoolFactory = NULL;
DynamicMemoryAllocator* TheDynamicMemoryAllocator = NULL;
CriticalSection* TheDmaCriticalSection = NULL;
CriticalSection* TheMemoryPoolCriticalSection = NULL;

static COMPILETIMEEVAL Int roundUpMemBound(Int i)
{
	return (i + (MEM_BOUND_ALIGNMENT - 1)) & ~(MEM_BOUND_ALIGNMENT - 1);
}

static NOINLINE void* sysAllocateDoNotZero(Int numBytes)
{
	void* p = ::GlobalAlloc(GMEM_FIXED, numBytes);
	if (!p)
		MessageBox(NULL, "GlobalAlloc failed", "Error", MB_OK);


	return p;
}

static NOINLINE void* sysAllocate(Int numBytes)
{
	void* p = ::GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, numBytes);

	if (!p)
		MessageBox(NULL, "GlobalAlloc failed", "Error", MB_OK);

	return p;
}

static void sysFree(void* p)
{
	if (p)
	{
		::GlobalFree(p);
	}
}

static void memset32(void* ptr, Int value, Int bytesToFill)
{
	Int wordsToFill = bytesToFill >> 2;
	bytesToFill -= (wordsToFill << 2);

	Int* p = (Int*)ptr;
	for (++wordsToFill; --wordsToFill; )
		*p++ = value;

	Byte* b = (Byte*)p;
	for (++bytesToFill; --bytesToFill; )
		*b++ = (Byte)value;
}

// And please be careful of duplicates.  They are not rejected.
// not const -- we might override from INI
static PoolSizeRec sizes[] =
{
	{ "AnimExtData"				, 9000	, 1024	},
	{ "BulletExtData"			, 9000	, 1024	},
	{ "BuildingExtData"			, 9000	, 1024	},
	{ "InfantryExtData"			, 9000	, 1024	},
	{ "ParticleExtData"			, 9000	, 1024	},
	{ "ParticleSystemExtData"	, 9000	, 1024	},
	{ "PhobosAttachEffectClass" , 9000	, 1024	},
	{ "RadSiteExtData"			, 100	, 50	},
	{ "ShieldClass"				, 1000	, 100	},
	{ "TeamExtData"				, 9000	, 1024	},
	{ "TechnoExtData"			, 9000	, 500	},
	{ "TerrainExtData"			, 500	, 100	},
	{ "TemporalExtData"			, 500	, 100	},
	{ "VoxelAnimExtData"		, 1000	, 100	},
	{ "WaveExtData"				, 100	, 50	},

	//SW StateMachines
	{ "ChronoWarpStateMachine"				, 100	, 50	},
	{ "CloneableLighningStormStateMachine"	, 100	, 50	},
	{ "DroppodStateMachine"					, 100	, 50	},
	{ "GeneticMutatorStateMachine"			, 100	, 50	},
	{ "GenericWarheadStateMachine"			, 100	, 50	},
	{ "ParaDropStateMachine"				, 100	, 50	},
	{ "PsychicDominatorStateMachine"		, 100	, 50	},
	{ "RevealStateMachine"					, 100	, 50	},
	{ "SonarPulseStateMachine"				, 100	, 50	},
	{ "SpyPlaneStateMachine"				, 100	, 50	},
	{ "UnitDeliveryStateMachine"			, 100	, 50	},
	{ "IonCannonStateMachine"				, 100	, 50	},
	{ "LaserStrikeStateMachine"				, 100	, 50	},

	{ "RadarJammerClass"					, 100	, 50	},
	{ "PoweredUnitClass"					, 100	, 50	},
	{ "PrismForwarding"					, 100	, 50	},
};

static void userMemoryAdjustPoolSize(const char* poolName, Int& initialAllocationCount, Int& overflowAllocationCount)
{
	if (initialAllocationCount > 0)
		return;

	for (const PoolSizeRec* p = sizes; p->name != NULL; ++p)
	{
		if (strcmp(p->name, poolName) == 0)
		{
			initialAllocationCount = p->initial;
			overflowAllocationCount = p->overflow;
			return;
		}
	}
}

void Mem::userMemoryManagerGetDmaParms(Int* numSubPools, const PoolInitRec** pParms)
{
	static const PoolInitRec defaultDMA[7] =
	{
		// name, allocsize, initialcount, overflowcount
		{ "dmaPool_16"	, 16	, 65536,	1024 },
		{ "dmaPool_32"	, 32	, 150000,	1024 },
		{ "dmaPool_64"	, 64	, 60000,	1024 },
		{ "dmaPool_128"	, 128	, 32768,	1024 },
		{ "dmaPool_256"	, 256	, 8192,		1024 },
		{ "dmaPool_512"	, 512	, 8192,		1024 },
		{ "dmaPool_1024", 1024	, 24000,	1024 }
	};

	*numSubPools = std::size(defaultDMA);
	*pParms = defaultDMA;
}

static Bool thePreMainInitFlag = false;
static Bool theMainInitFlag = false;

void Mem::preMainInitMemoryManager()
{
	if (TheMemoryPoolFactory == NULL)
	{
		Int numSubPools;
		const PoolInitRec* pParms;
		Mem::userMemoryManagerGetDmaParms(&numSubPools, &pParms);
		TheMemoryPoolFactory = new (::sysAllocate(sizeof MemoryPoolFactory)) MemoryPoolFactory;	// will throw on failure
		TheMemoryPoolFactory->init();	// will throw on failure

		TheDynamicMemoryAllocator = TheMemoryPoolFactory->createDynamicMemoryAllocator(numSubPools, pParms);	// will throw on failure
		Mem::userMemoryManagerInitPools();
		thePreMainInitFlag = true;
	}
}

void Mem::userMemoryManagerInitPools()
{
	// note that we MUST use stdio stuff here, and not the normal game file system
	// (with bigfile support, etc), because that relies on memory pools, which
	// aren't yet initialized properly! so rely ONLY on straight stdio stuff here.
	// (not even AsciiString. thanks.)

	// since we're called prior to main, the cur dir might not be what
	// we expect. so do it the hard way.
	char buf[_MAX_PATH];
	::GetModuleFileName(NULL, buf, sizeof(buf));
	char* pEnd = buf + strlen(buf);
	while (pEnd != buf)
	{
		if (*pEnd == '\\')
		{
			*pEnd = 0;
			break;
		}
		--pEnd;
	}
	strcat(buf, "\\Data\\INI\\MemoryPools.ini");

	FILE* fp = fopen(buf, "r");
	if (fp)
	{
		char poolName[256];
		int initial, overflow;
		while (fgets(buf, _MAX_PATH, fp))
		{
			if (buf[0] == ';')
				continue;
			if (sscanf(buf, "%s %d %d", poolName, &initial, &overflow) == 3)
			{
				for (PoolSizeRec* p = sizes; p->name != NULL; ++p)
				{
					if (_stricmp(p->name, poolName) == 0)
					{
						// currently, these must be multiples of 4. so round up.
						p->initial = roundUpMemBound(initial);
						p->overflow = roundUpMemBound(overflow);
						break;	// from for-p
					}
				}
			}
		}
		fclose(fp);
	}
}

void Mem::shutdownMemoryManager()
{
	if (!thePreMainInitFlag)
	{
		if (TheDynamicMemoryAllocator)
		{
			if (TheMemoryPoolFactory)
				TheMemoryPoolFactory->destroyDynamicMemoryAllocator(TheDynamicMemoryAllocator);
			TheDynamicMemoryAllocator = NULL;
		}

		if (TheMemoryPoolFactory)
		{
			// this is evil... since there is no 'placement delete' we must do this the hard way
			// and call the dtor directly. ordinarily this is heinous, but in this case we'll
			// make an exception.
			TheMemoryPoolFactory->~MemoryPoolFactory();
			::sysFree((void*)TheMemoryPoolFactory);
			TheMemoryPoolFactory = NULL;
		}
	}

	theMainInitFlag = false;
}

#pragma region MemoryPoolFactory

MemoryPoolFactory::MemoryPoolFactory() :
	m_firstPoolInFactory(NULL),
	m_firstDmaInFactory(NULL)
{ }

MemoryPoolFactory::~MemoryPoolFactory()
{
	while (m_firstPoolInFactory)
	{
		destroyMemoryPool(m_firstPoolInFactory);
	}

	while (m_firstDmaInFactory)
	{
		destroyDynamicMemoryAllocator(m_firstDmaInFactory);
	}
}

MemoryPool* MemoryPoolFactory::createMemoryPool(const PoolInitRec* parms)
{
	return createMemoryPool(parms->poolName, parms->allocationSize, parms->initialAllocationCount, parms->overflowAllocationCount);
}

MemoryPool* MemoryPoolFactory::createMemoryPool(const char* poolName, Int allocationSize, Int initialAllocationCount, Int overflowAllocationCount)
{
	MemoryPool* pool = this->findMemoryPool(poolName);
	if (pool)
	{
		return pool;
	}

	userMemoryAdjustPoolSize(poolName, initialAllocationCount, overflowAllocationCount);

	if (initialAllocationCount <= 0 || overflowAllocationCount < 0)
	{
		throw ERROR_OUT_OF_MEMORY;
	}

	pool = new (::sysAllocate(sizeof MemoryPool)) MemoryPool;	// will throw on failure
	pool->init(this, poolName, allocationSize, initialAllocationCount, overflowAllocationCount);	// will throw on failure

	pool->addToList(&m_firstPoolInFactory);

	return pool;
}

void MemoryPoolFactory::reportAllocation()
{
	for (MemoryPool* pool = this->m_firstPoolInFactory; pool; pool = pool->getNextPoolInList()) {
		Debug::LogInfo("{} MemoryPool size {}", pool->getPoolName(), pool->getAllocationSize());
	}
}

MemoryPool* MemoryPoolFactory::findMemoryPool(const char* poolName)
{
	for (MemoryPool* pool = this->m_firstPoolInFactory; pool; pool = pool->getNextPoolInList())
	{
		if (!strcmp(poolName, pool->getPoolName()))
		{
			return pool;
		}
	}
	return NULL;
}

void MemoryPoolFactory::destroyMemoryPool(MemoryPool* pMemoryPool)
{
	if (!pMemoryPool)
		return;

	pMemoryPool->removeFromList(&this->m_firstPoolInFactory);

	// this is evil... since there is no 'placement delete' we must do this the hard way
	// and call the dtor directly. ordinarily this is heinous, but in this case we'll
	// make an exception.
	pMemoryPool->~MemoryPool();
	::sysFree((void*)pMemoryPool);
}

DynamicMemoryAllocator* MemoryPoolFactory::createDynamicMemoryAllocator(Int numSubPools, const PoolInitRec pParms[])
{
	DynamicMemoryAllocator* dma = new (::sysAllocate(sizeof DynamicMemoryAllocator)) DynamicMemoryAllocator;	// will throw on failure
	dma->init(this, numSubPools, pParms);	// will throw on failure
	dma->addToList(&m_firstDmaInFactory);
	return dma;
}

void  MemoryPoolFactory::destroyDynamicMemoryAllocator(DynamicMemoryAllocator* dma)
{
	if (!dma)
		return;

	dma->removeFromList(&m_firstDmaInFactory);

	// this is evil... since there is no 'placement delete' we must do this the hard way
	// and call the dtor directly. ordinarily this is heinous, but in this case we'll
	// make an exception.
	dma->~DynamicMemoryAllocator();
	::sysFree((void*)dma);
}

void MemoryPoolFactory::reset()
{
	for (MemoryPool* pool = m_firstPoolInFactory; pool; pool = pool->getNextPoolInList())
	{
		pool->reset();
	}
	for (DynamicMemoryAllocator* dma = m_firstDmaInFactory; dma; dma = dma->getNextDmaInList())
	{
		dma->reset();
	}
}

#pragma endregion

#pragma region MemoryPoolSingleBlock

void* MemoryPoolSingleBlock::getUserDataNoDbg()
{
	char* p = ((char*)this) + sizeof(MemoryPoolSingleBlock);
	return (void*)p;
}

Int MemoryPoolSingleBlock::calcRawBlockSize(Int logicalSize)
{
	Int s = ::roundUpMemBound(logicalSize) + sizeof(MemoryPoolSingleBlock);
	return s;
}

MemoryPoolSingleBlock* MemoryPoolSingleBlock::rawAllocateSingleBlock(MemoryPoolSingleBlock** pRawListHead, Int logicalSize, MemoryPoolFactory* owningFactory)
{
	MemoryPoolSingleBlock* block = (MemoryPoolSingleBlock*)::sysAllocateDoNotZero(calcRawBlockSize(logicalSize));
	block->initBlock(logicalSize, NULL, owningFactory);
	block->setNextRawBlock(*pRawListHead);
	*pRawListHead = block;
	return block;
}

void MemoryPoolSingleBlock::removeBlockFromList(MemoryPoolSingleBlock** pHead)
{
	// this isn't very efficient, and may need upgrading... but to do so
	// would require adding a back link, so I'd rather do some testing
	// first to see if it's really a speed issue in practice. (the only place
	// this is used is when freeing 'raw' blocks allocated via the DMA).
	MemoryPoolSingleBlock* prev = NULL;

	for (MemoryPoolSingleBlock* cur = *pHead; cur; cur = cur->m_nextBlock)
	{
		if (cur == this)
		{
			if (prev)
			{
				prev->m_nextBlock = this->m_nextBlock;
			}
			else
			{
				*pHead = this->m_nextBlock;
			}
			break;
		}
		prev = cur;
	}
}

void MemoryPoolSingleBlock::initBlock(Int logicalSize, MemoryPoolBlob* owningBlob, MemoryPoolFactory* owningFactory)
{
	// Note that while it is OK for owningBlob to be null, it is NEVER ok
	// for owningFactory to be null.
	m_nextBlock = NULL;
	m_owningBlob = owningBlob;	// could be NULL
}

void* MemoryPoolSingleBlock::getUserData() { return getUserDataNoDbg(); }

MemoryPoolSingleBlock* MemoryPoolSingleBlock::recoverBlockFromUserData(void* pUserData)
{
	if (!pUserData)
		return NULL;

	char* p = ((char*)pUserData) - sizeof(MemoryPoolSingleBlock);
	MemoryPoolSingleBlock* block = (MemoryPoolSingleBlock*)p;
	return block;
}

MemoryPoolBlob* MemoryPoolSingleBlock::getOwningBlob() { return m_owningBlob; }
MemoryPoolSingleBlock* MemoryPoolSingleBlock::getNextFreeBlock() { return m_nextBlock; }
void  MemoryPoolSingleBlock::setNextFreeBlock(MemoryPoolSingleBlock* b) { this->m_nextBlock = b; }
MemoryPoolSingleBlock* MemoryPoolSingleBlock::getNextRawBlock() { return m_nextBlock; }
void  MemoryPoolSingleBlock::setNextRawBlock(MemoryPoolSingleBlock* b) { m_nextBlock = b; }

#pragma endregion

#pragma region MemoryPoolBlob

MemoryPoolBlob::MemoryPoolBlob() :
	m_owningPool(NULL),
	m_nextBlob(NULL),
	m_prevBlob(NULL),
	m_firstFreeBlock(NULL),
	m_usedBlocksInBlob(0),
	m_totalBlocksInBlob(0),
	m_blockData(NULL)
{ }

MemoryPoolBlob::~MemoryPoolBlob()
{
	::sysFree((void*)m_blockData);
}

void MemoryPoolBlob::initBlob(MemoryPool* owningPool, Int allocationCount)
{
	m_owningPool = owningPool;
	m_totalBlocksInBlob = allocationCount;
	m_usedBlocksInBlob = 0;

	Int rawBlockSize = MemoryPoolSingleBlock::calcRawBlockSize(m_owningPool->getAllocationSize());
	m_blockData = (char*)::sysAllocate(rawBlockSize * m_totalBlocksInBlob);	// throws on failure

	// set up the list of free blocks in the blob (namely, all of 'em)
	MemoryPoolSingleBlock* block = (MemoryPoolSingleBlock*)m_blockData;
	MemoryPoolSingleBlock* next;
	for (Int i = m_totalBlocksInBlob - 1; i >= 0; i--)
	{
		next = (MemoryPoolSingleBlock*)(((char*)block) + rawBlockSize);
		block->initBlock(m_owningPool->getAllocationSize(), this, owningPool->getOwningFactory());

		block->setNextFreeBlock((i > 0) ? next : NULL);
		block = next;
	}

	m_firstFreeBlock = (MemoryPoolSingleBlock*)m_blockData;
}

void MemoryPoolBlob::addBlobToList(MemoryPoolBlob** ppHead, MemoryPoolBlob** ppTail)
{
	m_prevBlob = *ppTail;
	m_nextBlob = NULL;

	if (*ppTail != NULL)
		(*ppTail)->m_nextBlob = this;

	if (*ppHead == NULL)
		*ppHead = this;

	*ppTail = this;
}

void MemoryPoolBlob::removeBlobFromList(MemoryPoolBlob** ppHead, MemoryPoolBlob** ppTail)
{
	if (*ppHead == this)
		*ppHead = this->m_nextBlob;
	else
		this->m_prevBlob->m_nextBlob = this->m_nextBlob;

	if (*ppTail == this)
		*ppTail = this->m_prevBlob;
	else
		this->m_nextBlob->m_prevBlob = this->m_prevBlob;
}

MemoryPoolBlob* MemoryPoolBlob::getNextInList() { return m_nextBlob; }
Bool MemoryPoolBlob::hasAnyFreeBlocks() { return m_firstFreeBlock != NULL; }

MemoryPoolSingleBlock* MemoryPoolBlob::allocateSingleBlock()
{
	MemoryPoolSingleBlock* block = m_firstFreeBlock;
	m_firstFreeBlock = block->getNextFreeBlock();
	++m_usedBlocksInBlob;
	return block;
}

void MemoryPoolBlob::freeSingleBlock(MemoryPoolSingleBlock* block)
{
	block->setNextFreeBlock(m_firstFreeBlock);
	m_firstFreeBlock = block;
	--m_usedBlocksInBlob;
}

MemoryPool* MemoryPoolBlob::getOwningPool() { return m_owningPool; }
Int MemoryPoolBlob::getFreeBlockCount() { return getTotalBlockCount() - getUsedBlockCount(); }
Int MemoryPoolBlob::getUsedBlockCount() { return m_usedBlocksInBlob; }
Int MemoryPoolBlob::getTotalBlockCount() { return m_totalBlocksInBlob; }

#pragma endregion

#pragma region MemoryPool

MemoryPoolBlob* MemoryPool::createBlob(Int allocationCount)
{
	MemoryPoolBlob* blob = new (::sysAllocate(sizeof MemoryPoolBlob)) MemoryPoolBlob;	// will throw on failure

	blob->initBlob(this, allocationCount);	// will throw on failure
	blob->addBlobToList(&m_firstBlob, &m_lastBlob);

	m_firstBlobWithFreeBlocks = blob;
	m_totalBlocksInPool += allocationCount;
	return blob;
}

Int MemoryPool::freeBlob(MemoryPoolBlob* blob)
{
	// save these for later...
	Int totalBlocksInBlob = blob->getTotalBlockCount();
	Int usedBlocksInBlob = blob->getUsedBlockCount();

	// this is really just an estimate... will be too small in debug mode.
	Int amtFreed = totalBlocksInBlob * getAllocationSize() + sizeof(MemoryPoolBlob);

	// de-link it from our list
	blob->removeBlobFromList(&m_firstBlob, &m_lastBlob);

	// ensure that the 'first free' blob is still a valid blob.
	// (doesn't need to actually have free blocks, just be a valid blob)
	if (m_firstBlobWithFreeBlocks == blob)
		m_firstBlobWithFreeBlocks = m_firstBlob;

	// this is evil... since there is no 'placement delete' we must do this the hard way
	// and call the dtor directly. ordinarily this is heinous, but in this case we'll
	// make an exception.
	blob->~MemoryPoolBlob();
	::sysFree((void*)blob);

	// finally... bookkeeping
	m_usedBlocksInPool -= usedBlocksInBlob;
	m_totalBlocksInPool -= totalBlocksInBlob;

	return amtFreed;
}

MemoryPool* MemoryPool::getNextPoolInList() { return m_nextPoolInFactory; };					///< return next pool in linked list

void MemoryPool::addToList(MemoryPool** pHead)				///< add this pool to head of the linked list
{
	this->m_nextPoolInFactory = *pHead;
	*pHead = this;
}

void MemoryPool::removeFromList(MemoryPool** pHead)	///< remove this pool from the linked list
{
	// this isn't very efficient, but then, we rarely remove pools...
	// usually only at shutdown. so don't bother optimizing.
	MemoryPool* prev = NULL;
	for (MemoryPool* cur = *pHead; cur; cur = cur->m_nextPoolInFactory)
	{
		if (cur == this)
		{
			if (prev)
			{
				prev->m_nextPoolInFactory = this->m_nextPoolInFactory;
			}
			else
			{
				*pHead = this->m_nextPoolInFactory;
			}
			break;
		}
		prev = cur;
	}
}

MemoryPool::MemoryPool() :
	m_factory(NULL),
	m_nextPoolInFactory(NULL),
	m_poolName(""),
	m_allocationSize(0),
	m_initialAllocationCount(0),
	m_overflowAllocationCount(0),
	m_usedBlocksInPool(0),
	m_totalBlocksInPool(0),
	m_peakUsedBlocksInPool(0),
	m_firstBlob(NULL),
	m_lastBlob(NULL),
	m_firstBlobWithFreeBlocks(NULL)
{ }

void MemoryPool::init(MemoryPoolFactory* factory, const char* poolName, Int allocationSize, Int initialAllocationCount, Int overflowAllocationCount)
{
	m_factory = factory;
	m_poolName = poolName;
	m_allocationSize = ::roundUpMemBound(allocationSize);	// round up to four-byte boundary
	m_initialAllocationCount = initialAllocationCount;
	m_overflowAllocationCount = overflowAllocationCount;
	m_usedBlocksInPool = 0;
	m_totalBlocksInPool = 0;
	m_peakUsedBlocksInPool = 0;
	m_firstBlob = NULL;
	m_lastBlob = NULL;
	m_firstBlobWithFreeBlocks = NULL;

	// go ahead and init the initial block here (will throw on failure)
	createBlob(m_initialAllocationCount);
}

MemoryPool::~MemoryPool()
{
	// toss everything. we could do this slightly more efficiently,
	// but not really worth the extra code to do so.
	while (m_firstBlob)
	{
		freeBlob(m_firstBlob);
	}
}

void* MemoryPool::allocateBlockImplementation()
{
	void* p = allocateBlockDoNotZeroImplementation();	// throws on failure
	memset(p, 0, getAllocationSize());
	return p;
}

void* MemoryPool::allocateBlockDoNotZeroImplementation()
{
	ScopedCriticalSection scopedCriticalSection(TheMemoryPoolCriticalSection);

	if (m_firstBlobWithFreeBlocks != NULL && !m_firstBlobWithFreeBlocks->hasAnyFreeBlocks())
	{
		// hmm... the current 'free' blob has nothing available. look and see if there
		// are any other existing blobs with freespace.
		MemoryPoolBlob* blob = m_firstBlob;
		for (; blob != NULL; blob = blob->getNextInList())
		{
			if (blob->hasAnyFreeBlocks())
				break;
		}

		// note that if we walk thru the list without finding anything, this will
		// reset m_firstBlobWithFreeBlocks to null and fall thru.
		m_firstBlobWithFreeBlocks = blob;
	}

	// OK, if we are here then we have no blobs with freespace... darn.
	// allocate an overflow block.
	if (m_firstBlobWithFreeBlocks == NULL)
	{
		if (m_overflowAllocationCount == 0)
		{
			throw ERROR_OUT_OF_MEMORY;	// this pool is not allowed to grow
		}
		else
		{
			createBlob(m_overflowAllocationCount); // throws on failure
		}
	}

	MemoryPoolBlob* blob = m_firstBlobWithFreeBlocks;
	MemoryPoolSingleBlock* block = blob->allocateSingleBlock();

	// bookkeeping
	++m_usedBlocksInPool;
	if (m_peakUsedBlocksInPool < m_usedBlocksInPool)
		m_peakUsedBlocksInPool = m_usedBlocksInPool;

	return block->getUserData();
}

void MemoryPool::freeBlock(void* pMem)
{
	if (!pMem)
		return;	// my, that was easy

	ScopedCriticalSection scopedCriticalSection(TheMemoryPoolCriticalSection);

	MemoryPoolSingleBlock* block = MemoryPoolSingleBlock::recoverBlockFromUserData(pMem);
	MemoryPoolBlob* blob = block->getOwningBlob();


	blob->freeSingleBlock(block);

	// if we want to free the blobs as they become empty, do that here.
	// normally we don't bother, but just in case this is ever desired, here's how you'd do it...
	//
	// if (blob->m_usedBlocksInBlob == 0)
	// {
	//	freeBlob(blob);
	//	return;
	//}

	if (!m_firstBlobWithFreeBlocks)
		m_firstBlobWithFreeBlocks = blob;

	// bookkeeping
	--m_usedBlocksInPool;
}

MemoryPoolFactory* MemoryPool::getOwningFactory() { return m_factory; }
const char* MemoryPool::getPoolName() { return m_poolName; }
Int MemoryPool::getAllocationSize() { return m_allocationSize; }
Int MemoryPool::getFreeBlockCount() { return getTotalBlockCount() - getUsedBlockCount(); }
Int MemoryPool::getUsedBlockCount() { return m_usedBlocksInPool; }
Int MemoryPool::getTotalBlockCount() { return m_totalBlocksInPool; }
Int MemoryPool::getPeakBlockCount() { return m_peakUsedBlocksInPool; }
Int MemoryPool::getInitialBlockCount() { return m_initialAllocationCount; }

Int MemoryPool::countBlobsInPool()
{
	Int blobs = 0;
	for (MemoryPoolBlob* blob = m_firstBlob; blob;)
	{
		++blobs;
		blob = blob->getNextInList();
	}
	return blobs;
}

Int MemoryPool::releaseEmpties()
{
	ScopedCriticalSection scopedCriticalSection(TheMemoryPoolCriticalSection);

	Int released = 0;

	for (MemoryPoolBlob* blob = m_firstBlob; blob;)
	{
		MemoryPoolBlob* pNext = blob->getNextInList();
		if (blob->getUsedBlockCount() == 0)
			released += freeBlob(blob);
		blob = pNext;
	}
	return released;
}

void MemoryPool::reset()
{
	ScopedCriticalSection scopedCriticalSection(TheMemoryPoolCriticalSection);

	// toss everything. we could do this slightly more efficiently,
	// but not really worth the extra code to do so.
	while (m_firstBlob)
	{
		freeBlob(m_firstBlob);
	}
	m_firstBlob = NULL;
	m_lastBlob = NULL;
	m_firstBlobWithFreeBlocks = NULL;

	init(m_factory, m_poolName, m_allocationSize, m_initialAllocationCount, m_overflowAllocationCount);	// will throw on failure

}

#pragma endregion

#pragma region MemoryPoolObject

void MemoryPoolObject::deleteInstance()
{
	if (this)
	{
		MemoryPool* pool = this->getObjectMemoryPool(); // save this, since the dtor will nuke our vtbl
		this->~MemoryPoolObject();	// it's virtual, so the right one will be called.
		pool->freeBlock((void*)this);
	}
}

#pragma endregion

#pragma region DynamicMemoryAllocator

MemoryPool* DynamicMemoryAllocator::findPoolForSize(Int allocSize)
{
	for (Int i = 0; i < m_numPools; i++)
	{
		if (allocSize <= m_pools[i]->getAllocationSize())
			return m_pools[i];
	}
	return NULL;
}

DynamicMemoryAllocator* DynamicMemoryAllocator::getNextDmaInList() { return m_nextDmaInFactory; }

void DynamicMemoryAllocator::addToList(DynamicMemoryAllocator** pHead)
{
	this->m_nextDmaInFactory = *pHead;
	*pHead = this;
}

void DynamicMemoryAllocator::removeFromList(DynamicMemoryAllocator** pHead)
{
	// this isn't very efficient, but then, we rarely remove these...
	// usually only at shutdown. so don't bother optimizing.
	DynamicMemoryAllocator* prev = NULL;
	for (DynamicMemoryAllocator* cur = *pHead; cur; cur = cur->m_nextDmaInFactory)
	{
		if (cur == this)
		{
			if (prev)
			{
				prev->m_nextDmaInFactory = this->m_nextDmaInFactory;
			}
			else
			{
				*pHead = this->m_nextDmaInFactory;
			}
			break;
		}
		prev = cur;
	}
}

DynamicMemoryAllocator::DynamicMemoryAllocator() :
	m_factory(NULL),
	m_nextDmaInFactory(NULL),
	m_numPools(0),
	m_usedBlocksInDma(0),
	m_rawBlocks(NULL)
{
	for (Int i = 0; i < MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS; i++)
		m_pools[i] = 0;
}

void DynamicMemoryAllocator::init(MemoryPoolFactory* factory, Int numSubPools, const PoolInitRec pParms[])
{
	constexpr PoolInitRec defaultDMA[7] =
	{
		{ "dmaPool_16", 16, 64, 64 },
		{ "dmaPool_32", 32, 64, 64 },
		{ "dmaPool_64", 64, 64, 64 },
		{ "dmaPool_128", 128, 64, 64 },
		{ "dmaPool_256", 256, 64, 64 },
		{ "dmaPool_512", 512, 64, 64 },
		{ "dmaPool_1024", 1024, 64, 64 }
	};

	if (numSubPools == 0 || pParms == NULL)
	{
		// use the defaults...
		numSubPools = std::size(defaultDMA);
		pParms = defaultDMA;
	}

	m_factory = factory;
	m_numPools = numSubPools;
	if (m_numPools > MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS)
		m_numPools = MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS;
	m_usedBlocksInDma = 0;

	for (Int i = 0; i < m_numPools; i++)
	{
		m_pools[i] = m_factory->createMemoryPool(&pParms[i]);
	}
}

DynamicMemoryAllocator::~DynamicMemoryAllocator()
{
	/// @todo this may cause double-destruction of the subpools -- test & fix
	for (Int i = 0; i < m_numPools; i++)
	{
		m_factory->destroyMemoryPool(m_pools[i]);
		m_pools[i] = NULL;
	}

	while (m_rawBlocks)
	{
		freeBytes(m_rawBlocks->getUserData());
	}
}

void* DynamicMemoryAllocator::allocateBytesImplementation(Int numBytes)
{
	void* p = allocateBytesDoNotZeroImplementation(numBytes);	// throws on failure
	memset(p, 0, numBytes);
	return p;
}

void* DynamicMemoryAllocator::allocateBytesDoNotZeroImplementation(Int numBytes)
{
	ScopedCriticalSection scopedCriticalSection(TheDmaCriticalSection);

	void* result = NULL;

	MemoryPool* pool = findPoolForSize(numBytes);
	if (pool != NULL)
	{
		result = pool->allocateBlockDoNotZeroImplementation();
	}
	else
	{
		// too big for our pools -- just go right to the metal.
		MemoryPoolSingleBlock* block = MemoryPoolSingleBlock::rawAllocateSingleBlock(&m_rawBlocks, numBytes, m_factory);
		result = block->getUserData();
	}

	++m_usedBlocksInDma;
	return result;
}

void DynamicMemoryAllocator::freeBytes(void* pMem)
{
	if (!pMem)
		return;

	ScopedCriticalSection scopedCriticalSection(TheDmaCriticalSection);
	MemoryPoolSingleBlock* block = MemoryPoolSingleBlock::recoverBlockFromUserData(pMem);

	if (block->getOwningBlob())
	{
		block->getOwningBlob()->getOwningPool()->freeBlock(pMem);
	}
	else
	{
		// was allocated via sysAllocate.
		block->removeBlockFromList(&m_rawBlocks);

		::sysFree((void*)block);

	}
	--m_usedBlocksInDma;
}

Int DynamicMemoryAllocator::getActualAllocationSize(Int numBytes)
{
	MemoryPool* pool = findPoolForSize(numBytes);
	return pool ? pool->getAllocationSize() : numBytes;
}

void DynamicMemoryAllocator::reset()
{
	for (Int i = 0; i < m_numPools; i++)
	{
		if (m_pools[i])
		{
			m_pools[i]->reset();
		}
	}

	while (m_rawBlocks)
		freeBytes(m_rawBlocks->getUserData());

	m_usedBlocksInDma = 0;
}

Int DynamicMemoryAllocator::getDmaMemoryPoolCount() const { return m_numPools; }
MemoryPool* DynamicMemoryAllocator::getNthDmaMemoryPool(Int i) const { return m_pools[i]; }
#pragma endregion

#ifdef _ReplaceAlloc
void* __CRTDECL operator new(size_t size)
{
	Mem::preMainInitMemoryManager();
	return TheDynamicMemoryAllocator->allocateBytesImplementation(size);
}

void __CRTDECL operator delete(void* p) noexcept
{
	Mem::preMainInitMemoryManager();
	TheDynamicMemoryAllocator->freeBytes(p);
}

void* __CRTDECL operator new[](size_t size)
{
	Mem::preMainInitMemoryManager();
	return TheDynamicMemoryAllocator->allocateBytesImplementation(size);
}

void __CRTDECL operator delete[](void* p) noexcept
{
	Mem::preMainInitMemoryManager();
	TheDynamicMemoryAllocator->freeBytes(p);
}

// additional overloads to account for VC/MFC funky versions
void* __CRTDECL operator new(size_t size, const char*, int) noexcept
{
	Mem::preMainInitMemoryManager();
	return TheDynamicMemoryAllocator->allocateBytesImplementation(size);
}

void __CRTDECL operator delete(void* p, const char*, int) noexcept
{
	Mem::preMainInitMemoryManager();
	TheDynamicMemoryAllocator->freeBytes(p);
}

void* __CRTDECL operator new(std::size_t size, std::nothrow_t const& tag) noexcept
{
	Mem::preMainInitMemoryManager();
	return TheDynamicMemoryAllocator->allocateBytesImplementation(size);
}

void* __CRTDECL operator new[](std::size_t size, std::nothrow_t const& tag) noexcept
{
	Mem::preMainInitMemoryManager();
	return TheDynamicMemoryAllocator->allocateBytesImplementation(size);
}

void  __CRTDECL operator delete(void* p, std::nothrow_t const& tag) noexcept
{
	Mem::preMainInitMemoryManager();
	TheDynamicMemoryAllocator->freeBytes(p);
}

void  __CRTDECL operator delete[](void* p, std::nothrow_t const& tag) noexcept
{
	Mem::preMainInitMemoryManager();
	TheDynamicMemoryAllocator->freeBytes(p);
}

void* __CRTDECL operator new[](size_t size, const char*, int) noexcept
{
	Mem::preMainInitMemoryManager();
	return TheDynamicMemoryAllocator->allocateBytesImplementation(size);
}

void __CRTDECL operator delete[](void* p, const char*, int) noexcept
{
	Mem::preMainInitMemoryManager();
	TheDynamicMemoryAllocator->freeBytes(p);
}

__declspec(restrict)void* __cdecl __malloc(std::size_t size)
{
	Mem::preMainInitMemoryManager();
	return TheDynamicMemoryAllocator->allocateBytesDoNotZero(size);
}

void __cdecl __free(void* ptr)
{
	Mem::preMainInitMemoryManager();
	TheDynamicMemoryAllocator->freeBytes(ptr);
}

std::size_t custom_msize(void* ptr)
{
	if (!ptr) return 0;
	return *((std::size_t*)ptr - 1);
}

__declspec(restrict) void* __cdecl __realloc(void* ptr, std::size_t size)
{
	Mem::preMainInitMemoryManager();
	if (!ptr)
	{
		// realloc(NULL, size) is same as malloc(size)
		return TheDynamicMemoryAllocator->allocateBytesDoNotZero(size);
	}

	if (size == 0)
	{
		// realloc(ptr, 0) is same as free(ptr)
		TheDynamicMemoryAllocator->freeBytes(ptr);
		return nullptr;
	}

	// Get the current size of the memory block
	std::size_t old_size = custom_msize(ptr);

	// Allocate new memory
	void* new_ptr = TheDynamicMemoryAllocator->allocateBytesDoNotZero(size);
	if (!new_ptr) return nullptr; // Allocation failed

	// Copy over the smaller of old/new size
	std::memcpy(new_ptr, ptr, old_size < size ? old_size : size);

	// Free the old block
	TheDynamicMemoryAllocator->freeBytes(ptr);

	return new_ptr;
}

__declspec(restrict) void* __cdecl __calloc(std::size_t count, std::size_t size)
{
	Mem::preMainInitMemoryManager();
	std::size_t total_size = count * size;

	// Optional: check for overflow
	if (size != 0 && total_size / size != count)
	{
		// Overflow detected
		return nullptr;
	}

	void* ptr = TheDynamicMemoryAllocator->allocateBytesDoNotZero(size);
	if (!ptr) return nullptr;

	std::memset(ptr, 0, total_size);
	return ptr;
}

#pragma comment(linker, "/alternatename:malloc=__malloc")
#pragma comment(linker, "/alternatename:free=__free")
#pragma comment(linker, "/alternatename:realloc=__realloc")
#pragma comment(linker, "/alternatename:calloc=__calloc")
#endif