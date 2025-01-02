#include "MemoryDescriptor.h"
#include "Kernel.h"
#include "Machine.h"
#include "PageDirectory.h"
#include "PageManager.h"
#include "PageTable.h"
#include "Utility.h"
#include "Video.h"

void MemoryDescriptor::Initialize() {
  KernelPageManager &kernelPageManager =
      Kernel::Instance().GetKernelPageManager();

  /* m_UserPageTableArray需要把AllocMemory()返回的物理内存地址 + 0xC0000000 */
  this->m_UserPageTableArray =
      (PageTable *)(kernelPageManager.AllocMemory(sizeof(PageTable) *
                                                  USER_SPACE_PAGE_TABLE_CNT) +
                    Machine::KERNEL_SPACE_START_ADDRESS);
}

void MemoryDescriptor::Release() {
  KernelPageManager &kernelPageManager =
      Kernel::Instance().GetKernelPageManager();
  if (this->m_UserPageTableArray) {
    kernelPageManager.FreeMemory(sizeof(PageTable) * USER_SPACE_PAGE_TABLE_CNT,
                                 (unsigned long)this->m_UserPageTableArray -
                                     Machine::KERNEL_SPACE_START_ADDRESS);
    this->m_UserPageTableArray = NULL;
  }
}

unsigned int MemoryDescriptor::MapEntry(unsigned long virtualAddress,
                                        unsigned int size,
                                        unsigned long phyPageIdx,
                                        bool isReadWrite) {
  unsigned long address = virtualAddress - USER_SPACE_START_ADDRESS;

  // 计算从pagetable的哪一个地址开始映射
  unsigned long startIdx = address >> 12;
  unsigned long cnt =
      (size + (PageManager::PAGE_SIZE - 1)) / PageManager::PAGE_SIZE;

  PageTableEntry *entrys = (PageTableEntry *)this->m_UserPageTableArray;
  for (unsigned int i = startIdx; i < startIdx + cnt; i++, phyPageIdx++) {
    entrys[i].m_Present = 0x1;
    entrys[i].m_ReadWriter = isReadWrite;
    entrys[i].m_PageBaseAddress = phyPageIdx;
  }
  return phyPageIdx;
}

void MemoryDescriptor::MapTextEntrys(unsigned long textStartAddress,
                                     unsigned long textSize,
                                     unsigned long textPageIdx) {
  this->MapEntry(textStartAddress, textSize, textPageIdx, false);
}
void MemoryDescriptor::MapDataEntrys(unsigned long dataStartAddress,
                                     unsigned long dataSize,
                                     unsigned long dataPageIdx) {
  this->MapEntry(dataStartAddress, dataSize, dataPageIdx, true);
}

void MemoryDescriptor::MapStackEntrys(unsigned long stackSize,
                                      unsigned long stackPageIdx) {
  unsigned long stackStartAddress =
      (USER_SPACE_START_ADDRESS + USER_SPACE_SIZE - stackSize) & 0xFFFFF000;
  this->MapEntry(stackStartAddress, stackSize, stackPageIdx, true);
}

PageTable *MemoryDescriptor::GetUserPageTableArray() {
  return this->m_UserPageTableArray;
}
unsigned long MemoryDescriptor::GetTextStartAddress() {
  return this->m_TextStartAddress;
}
unsigned long MemoryDescriptor::GetTextSize() { return this->m_TextSize; }
unsigned long MemoryDescriptor::GetDataStartAddress() {
  return this->m_DataStartAddress;
}
unsigned long MemoryDescriptor::GetDataSize() { return this->m_DataSize; }
unsigned long MemoryDescriptor::GetStackSize() { return this->m_StackSize; }

bool MemoryDescriptor::EstablishUserPageTable(unsigned long textVirtualAddress,
                                              unsigned long textSize,
                                              unsigned long dataVirtualAddress,
                                              unsigned long dataSize,
                                              unsigned long stackSize) {
  User &u = Kernel::Instance().GetUser();

  /* 如果超出允许的用户程序最大8M的地址空间限制 */
  if (textSize + dataSize + stackSize + PageManager::PAGE_SIZE >
      USER_SPACE_SIZE - textVirtualAddress) {
    u.u_error = User::ENOMEM;
    Diagnose::Write("u.u_error = %d\n", u.u_error);
    return false;
  }

  this->ClearUserPageTable();

  /* 以相对起始地址phyPageIndex为0，为正文段建立相对地址映照表 */
  unsigned int phyPageIndex = 0;
  phyPageIndex =
      this->MapEntry(textVirtualAddress, textSize, phyPageIndex, false);

  /* 以相对起始地址phyPageIndex为1，ppda区占用1页4K大小物理内存，为数据段建立相对地址映照表
   */
  phyPageIndex = 1;
  phyPageIndex =
      this->MapEntry(dataVirtualAddress, dataSize, phyPageIndex, true);

  /* 紧跟着数据段之后，为堆栈段建立相对地址映照表 */
  unsigned long stackStartAddress =
      (USER_SPACE_START_ADDRESS + USER_SPACE_SIZE - stackSize) & 0xFFFFF000;
  this->MapEntry(stackStartAddress, stackSize, phyPageIndex, true);

  /* 将相对地址映照表根据正文段和数据段在内存中的起始地址pText->x_caddr、p_addr，建立用户态内存区的页表映射
   */
  this->MapToPageTable();
  return true;
}

void MemoryDescriptor::ClearUserPageTable() {
  User &u = Kernel::Instance().GetUser();
  PageTable *pUserPageTable = u.u_MemoryDescriptor.m_UserPageTableArray;

  unsigned int i;
  unsigned int j;

  for (i = 0; i < Machine::USER_PAGE_TABLE_CNT; i++) {
    for (j = 0; j < PageTable::ENTRY_CNT_PER_PAGETABLE; j++) {
      pUserPageTable[i].m_Entrys[j].m_Present = 0;
      pUserPageTable[i].m_Entrys[j].m_ReadWriter = 0;
      pUserPageTable[i].m_Entrys[j].m_UserSupervisor = 1;
      pUserPageTable[i].m_Entrys[j].m_PageBaseAddress = 0;
    }
  }
}

static void MapTableAssert(bool const expr) {
  if (!expr) {
    Utility::Panic("Assertion Failed");
  }
}

void MemoryDescriptor::MapToPageTable() {
  User &u = Kernel::Instance().GetUser();

  if (u.u_MemoryDescriptor.m_UserPageTableArray == NULL)
    return;

  PageTable *pUserPageTable = Machine::Instance().GetUserPageTableArray();
  unsigned int textAddress = 0;
  if (u.u_procp->p_textp != NULL) {
    textAddress = u.u_procp->p_textp->x_caddr;
  }

  // NOTE: 首先进行存页表所有存在位的清零
  for (unsigned i = 0; i < Machine::USER_PAGE_TABLE_CNT; i++) {
    for (unsigned j = 0; j < PageTable::ENTRY_CNT_PER_PAGETABLE; j++) {
      pUserPageTable[i].m_Entrys[j].m_Present = 0;
    }
  }

  // NOTE: 第一部分, 代码段, 只需要关注PageTable1
  unsigned const text_count = this->GetTextSize() >> 12; // 一共有这么多组text
  // 计算的时候从1开始,因为0项是保留的,最大值也并不是1024, 而是1+text_count
  for (unsigned i = 1, j = 0; i < text_count + 1; i++, j++) {
    pUserPageTable[1].m_Entrys[i].m_Present = 1;
    pUserPageTable[1].m_Entrys[i].m_ReadWriter = 0; // 代码段只读
    pUserPageTable[1].m_Entrys[i].m_PageBaseAddress = (textAddress >> 12) + j;
  }
  Diagnose::Write("Written text segment[%d]-[%d]\n", this->GetTextSize(),
                  text_count);
  // NOTE: 第二部分，数据段, 只需关注PageTable1
  unsigned const data_count = this->GetDataSize() >> 12;
  // 计算的时候紧接着从上一次的位置开始, 但是偏移量从j=1开始,
  // 一直到j = data_count;
  unsigned const start_pos = text_count + 1;
  for (unsigned i = start_pos, j = 1; i < start_pos + data_count; i++, j++) {
    pUserPageTable[1].m_Entrys[i].m_Present = 1;
    pUserPageTable[1].m_Entrys[i].m_ReadWriter = 1; // 堆栈段可读可写
    pUserPageTable[1].m_Entrys[i].m_PageBaseAddress =
        (u.u_procp->p_addr >> 12) + j;
    MapTableAssert(j <= data_count);
  }
  Diagnose::Write("Written data segment[%d]-[%d]\n", this->GetDataSize(),
                  data_count);
  // NOTE: 最后一个项, 对应堆栈段
  unsigned const pos = PageTable::ENTRY_CNT_PER_PAGETABLE - 1;
  pUserPageTable[1].m_Entrys[pos].m_Present = 1;
  pUserPageTable[1].m_Entrys[pos].m_ReadWriter = 1;
  pUserPageTable[1].m_Entrys[pos].m_PageBaseAddress =
      (u.u_procp->p_addr >> 12) + data_count + 1;

  // for (unsigned int i = 0; i < Machine::USER_PAGE_TABLE_CNT; i++) {
  //   for (unsigned int j = 0; j < PageTable::ENTRY_CNT_PER_PAGETABLE; j++) {
  //     pUserPageTable[i].m_Entrys[j].m_Present = 0; // 先清0
  //     //
  //
  //     if (1 == this->m_UserPageTableArray[i].m_Entrys[j].m_Present) {
  //       /* 只读属性表示正文段对应的页，以pText->x_caddr为内存起始地址 */
  //       if (0 == this->m_UserPageTableArray[i].m_Entrys[j].m_ReadWriter) {
  //         pUserPageTable[i].m_Entrys[j].m_Present = 1;
  //         pUserPageTable[i].m_Entrys[j].m_ReadWriter =
  //             this->m_UserPageTableArray[i].m_Entrys[j].m_ReadWriter;
  //         pUserPageTable[i].m_Entrys[j].m_PageBaseAddress =
  //             this->m_UserPageTableArray[i].m_Entrys[j].m_PageBaseAddress +
  //             (textAddress >> 12);
  //       }
  //       /* 读写属性表示数据段对应的页，以p_addr为内存起始地址 */
  //       else if (1 == this->m_UserPageTableArray[i].m_Entrys[j].m_ReadWriter)
  //       {
  //         pUserPageTable[i].m_Entrys[j].m_Present = 1;
  //         pUserPageTable[i].m_Entrys[j].m_ReadWriter =
  //             this->m_UserPageTableArray[i].m_Entrys[j].m_ReadWriter;
  //         pUserPageTable[i].m_Entrys[j].m_PageBaseAddress =
  //             this->m_UserPageTableArray[i].m_Entrys[j].m_PageBaseAddress +
  //             (u.u_procp->p_addr >> 12);
  //       }
  //     }
  //   }
  // }
  //
  pUserPageTable[0].m_Entrys[0].m_Present = 1;
  pUserPageTable[0].m_Entrys[0].m_ReadWriter = 1;
  pUserPageTable[0].m_Entrys[0].m_PageBaseAddress = 0;

  FlushPageDirectory();
}
