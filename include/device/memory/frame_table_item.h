//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_FRAME_TABLE_ITEM_H
#define SDOS_FRAME_TABLE_ITEM_H

class FrameTableItem {
private:
    unsigned long mFrameAddress;
    unsigned long mLogicPage;
    unsigned int  mPid;
    bool          mOccupied;
public:
    explicit FrameTableItem(unsigned long frameAddress);
    bool operator == (const FrameTableItem&);
    unsigned int getPid() { return mPid; }
    void clear();
    bool isOccupiedBy(unsigned int pid, unsigned long logicalPage);
    bool occupy(unsigned int pid, unsigned long logicalPage);
    bool isOccupied() { return mOccupied; }
    unsigned long getFrameAddress() { return mFrameAddress; }
    void updateFrame(unsigned long frameAddress, unsigned long logicalPage, unsigned int pid);
    void setFrameAddress(unsigned long frameAddress);
};

#endif //SDOS_FRAME_TABLE_ITEM_H
