// SPDX-License-Identifier: MulanPSL-2.0
// https://github.com/FlowerBlackG/YurongOS/blob/master/src/video/svga/svga.cpp

/*
 * SVGA
 *
 * ������ 2023��7��5�� �Ϻ��мζ�����ͤ��
 */

#ifdef USE_VESA

#include "./svga.h"

#include "Machine.h"
#include "PageManager.h"

namespace video {
namespace svga {


int8_t* screen;
int8_t bytesPerPixel;
VbeModeInfo* modeInfo;
int32_t videoMemSize;
int32_t vMemPages;

void init(VbeModeInfo* modeInfo) {
    svga::modeInfo = modeInfo;
    svga::screen = (int8_t*) (VESA_SCREEN_VADDR);
    bytesPerPixel = modeInfo->bpp / 8;
    videoMemSize = bytesPerPixel * modeInfo->width * modeInfo->height;
    vMemPages = (videoMemSize + PageManager::PAGE_SIZE - 1) / PageManager::PAGE_SIZE;
}



}
}

#endif