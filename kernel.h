#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdbool.h>
#include <stdint.h>

// wrapper to mkfs
//   pass the name you want for the formatted disk/SD card in name, and the drive number (0-2) in the drive param.
//   do NOT prepend the path onto name. 
// return negative number on any error
int __fastcall__ mkfs(const char* name, const char drive);

char GETIN(void);
void kernel_init(void);

// runs a name program (a KUP, in other words)
// pass the KUP name and length
// returns error on error, and never returns on success (because SuperBASIC took over)
void Kernal_RunNamed(char* kup_name, uint8_t name_len);

// check for any kernel key press. return true if any key was pressed, otherwise false
// NOTE: the key press in question will be lost! only use when you want to check, but not wait for, a user key press
bool Kernal_AnyKeyEvent();

void out(char c);

#endif /* KERNEL_H_ */
