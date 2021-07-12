#ifndef DRAWARRAYSCOMMAND_H
#define DRAWARRAYSCOMMAND_H

struct DrawArraysCommand
{
  uint count;
  uint instanceCount;
  uint first;
  uint baseInstance;
};

#endif // DRAWARRAYSCOMMAND_H