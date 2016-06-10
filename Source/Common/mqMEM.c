#include "config.h"

// Local defines
typedef struct S_BLOCK_LINK
{
    struct  S_BLOCK_LINK *pNext;
    size_t  BlockSize;
} BlockLink_t;

#if    (portBYTE_ALIGNMENT == 8)
    #define portBYTE_ALIGNMENT_MASK     (0x0007U)
#elif  (portBYTE_ALIGNMENT == 4)
    #define portBYTE_ALIGNMENT_MASK     (0x0003)
#elif (portBYTE_ALIGNMENT == 2)
    #define portBYTE_ALIGNMENT_MASK     (0x0001)
#elif (portBYTE_ALIGNMENT == 1)
    #define portBYTE_ALIGNMENT_MASK     (0x0000)
#else
    #error "Invalid portBYTE_ALIGNMENT definition"
#endif  //  portBYTE_ALIGNMENT

#define configADJUSTED_HEAP_SIZE    (configTOTAL_HEAP_SIZE - portBYTE_ALIGNMENT)
#define heapSTRUCT_SIZE ((uint16_t)((sizeof(BlockLink_t) + (portBYTE_ALIGNMENT - 1)) &  \
                                                           ~portBYTE_ALIGNMENT_MASK))

// Allocate the memory for the heap.
static uint8_t mqMemHeap[configTOTAL_HEAP_SIZE];
// Create a couple of list links to mark the start and end of the list.
static BlockLink_t mqMemStart, mqMemEnd;

#ifdef DIAG_USED
static size_t mqHeapAct, mqHeapMin, mqHeapMax;
#endif  // DIAG_USED


void mqInit(void)
{
    BlockLink_t * pFirstFreeBlock;
    uint8_t     * pAlignedHeap;

    // Ensure the heap starts on a correctly aligned boundary.
    pAlignedHeap = (uint8_t *)(((portPOINTER_SIZE_TYPE)&mqMemHeap[portBYTE_ALIGNMENT]) & 
                               ((portPOINTER_SIZE_TYPE)~portBYTE_ALIGNMENT_MASK));

    // mqMemStart is used to hold a pointer to the first item in the list of free blocks.
    mqMemStart.pNext = (void *)pAlignedHeap;
    mqMemStart.BlockSize = 0;

    // mqMemEnd is used to mark the end of the list of free blocks.
    mqMemEnd.pNext = NULL;
    mqMemEnd.BlockSize = configADJUSTED_HEAP_SIZE;

    // To start with there is a single free block that is sized to take up the entire heap space.
    pFirstFreeBlock = (void *)pAlignedHeap;
    pFirstFreeBlock->BlockSize = configADJUSTED_HEAP_SIZE;
    pFirstFreeBlock->pNext = &mqMemEnd;
    
#ifdef DIAG_USED
    mqHeapAct = 0;
    mqHeapMin = 0;
    mqHeapMax = 0;
#endif  // DIAG_USED
}

void * mqAlloc(size_t xWantedSize)
{
    BlockLink_t *pBlock, *pPrevBlock, *pNewBlock, *pIterator;
    void * pReturn = NULL;

    ENTER_CRITICAL_SECTION();

    // The wanted size is increased so it can contain a BlockLink_t structure 
    //      in addition to the requested amount of bytes.
    xWantedSize += heapSTRUCT_SIZE;

    // Ensure that blocks are always aligned to the required number of bytes.
    if((xWantedSize & portBYTE_ALIGNMENT_MASK) != 0)
    {
        // Byte alignment required.
        xWantedSize += (portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK));
    }

    if(xWantedSize >= configADJUSTED_HEAP_SIZE)     // memory Overflow
        HAL_Reboot();

    // Blocks are stored in byte order - traverse the list from the start
    //  (smallest) block until one of adequate size is found.
    pPrevBlock = &mqMemStart;
    pBlock = mqMemStart.pNext;
    while( ( pBlock->BlockSize < xWantedSize ) && ( pBlock->pNext != NULL ) )
    {
        pPrevBlock = pBlock;
        pBlock = pBlock->pNext;
    }

    // If we found the end marker then a block of adequate size was not found.
    if(pBlock != &mqMemEnd)
    {
        // Return the memory space - jumping over the BlockLink_t structure at its start.
        pReturn = (void *)(((uint8_t *)pPrevBlock->pNext) + heapSTRUCT_SIZE);

        // This block is being returned for use so must be taken out of the list of free blocks.
        pPrevBlock->pNext = pBlock->pNext;

        // If the block is larger than required it can be split into two.
        if((pBlock->BlockSize - xWantedSize) > ((size_t)(heapSTRUCT_SIZE * 2)))
        {
            // This block is to be split into two.
            // Create a new block following the number of bytes requested. 
            pNewBlock = (void *)(((uint8_t *)pBlock) + xWantedSize);

            // Calculate the sizes of two blocks split from the single block.
            pNewBlock->BlockSize = pBlock->BlockSize - xWantedSize;
            pBlock->BlockSize = xWantedSize;

            // Insert the new block into the list of free blocks.
            size_t BlockSize = pNewBlock->BlockSize;
            for(pIterator = &mqMemStart;
                pIterator->pNext->BlockSize < BlockSize;
                pIterator = pIterator->pNext);

            pNewBlock->pNext = pIterator->pNext;
            pIterator->pNext = pNewBlock;
        }
#ifdef DIAG_USED
        mqHeapAct += pBlock->BlockSize;
        if(mqHeapMax < mqHeapAct)
            mqHeapMax = mqHeapAct;
#endif  //  DIAG_USED
    }
    else    //  Memory overflow
        HAL_Reboot();

    LEAVE_CRITICAL_SECTION();

    return pReturn;
}

void mqFree(void *pBuf)
{
    BlockLink_t *pLink, *pIterator;

    if(pBuf != NULL)
    {
        uint8_t *puc = (uint8_t *)pBuf;
    
        /* The memory being freed will have an BlockLink_t structure immediately
        before it. */
        puc -= heapSTRUCT_SIZE;

        /* This unexpected casting is to keep some compilers from issuing
        byte alignment warnings. */
        pLink = (void *)puc;

        ENTER_CRITICAL_SECTION();
        
        // Add this block to the list of free blocks.
        size_t BlockSize = pLink->BlockSize;
        for(pIterator = &mqMemStart;
            pIterator->pNext->BlockSize < BlockSize;
            pIterator = pIterator->pNext);
        pLink->pNext = pIterator->pNext;
        pIterator->pNext = pLink;

#ifdef DIAG_USED
        mqHeapAct -= pLink->BlockSize;
        if(mqHeapMin > mqHeapAct)
            mqHeapMin = mqHeapAct;
#endif  //  DIAG_USED
        
        LEAVE_CRITICAL_SECTION();
    }
}

#ifdef DIAG_USED
void mqGetHeapStat(uint16_t *pAct, uint16_t *pMax, uint16_t *pMin)
{
    *pAct = mqHeapAct;
    *pMax = mqHeapMax;
    *pMin = mqHeapMin;
    
    mqHeapMax = mqHeapAct;
    mqHeapMin = mqHeapAct;
}
#endif  //  DIAG_USED


/*
Queue_t * MEM_Create_Queue(uint8_t mSize)
{
    Queue_t * pQueue;
    pQueue = mqAlloc(sizeof(Queue_t));
    if(pQueue != NULL)
    {
        pQueue->pHead   = NULL;
        pQueue->pTail   = NULL;
        pQueue->MaxSize = mSize;
        pQueue->Size    = 0;
    }

    return pQueue;
}
*/

bool mqEnqueue(Queue_t * pQueue, void * pBuf)
{
    if((pQueue == NULL) || (pBuf == NULL))
        return false;

    ENTER_CRITICAL_SECTION();

    ((MQ_t *)pBuf)->pNext = NULL;
    
    if(pQueue->pHead == NULL)       // 1st element
    {
        pQueue->pHead = pBuf;
        pQueue->pTail = pBuf;
        pQueue->Size = 1;
    }
    else
    {
        if(pQueue->MaxSize != 0)
        {
            if((pQueue->Size + 1) > pQueue->MaxSize)
            {
                LEAVE_CRITICAL_SECTION();
                return false;
            }
        }

        MQ_t * pTmp;
        pTmp = pQueue->pHead;
        pTmp->pNext = pBuf;
        pQueue->pHead = pBuf;
        pQueue->Size++;
    }

    LEAVE_CRITICAL_SECTION();
    return true;
}

void * mqDequeue(Queue_t * pQueue)
{
    if(pQueue == NULL)
        return NULL;

    ENTER_CRITICAL_SECTION();

    MQ_t * pBuf = pQueue->pTail;

    if(pBuf != NULL)
    {
        if(pQueue->Size > 0)
            pQueue->Size--;

        pQueue->pTail = pBuf->pNext;
        if(pQueue->pTail == NULL)
        {
            pQueue->pHead = NULL;
            pQueue->Size = 0;
        }
    }

    LEAVE_CRITICAL_SECTION();
    return pBuf;
}
