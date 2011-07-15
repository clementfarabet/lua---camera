//==============================================================================
// File: camiface
//
// Description: this code grabs grames from a system-wide frame server, via
//              os-level shared memory
//
// Created: July  3, 2011, 11:31PM
//
// Author: Clement Farabet // clement.farabet@gmail.com
//==============================================================================

#include <dirent.h>
#include <errno.h>

#include <luaT.h>
#include <TH.h>

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <pthread.h>

// Data struct used to represent the shared segment
struct video_buffer {
  char request_frame;
  char frame_ready;
  char bytes_per_pixel;
  char exit;
  int width;
  int height;
  unsigned char data[];
};

static volatile struct video_buffer * buffer = NULL; 
static int shared_mem_connected = 0;
static key_t shmem_key;
static int shmem_id;  

// To exec in lua: sys.connectSharedMemory('path')
static int l_getSharedFrame (lua_State *L) {
  // Shared Mem Info
  const char *path = luaL_checkstring(L, 1);
  int exit = 0;

  // Dump ?
  if (lua_isboolean(L, 3)) exit = lua_toboolean(L, 3);

  // Get Tensor's Info
  THDoubleTensor * tensor64 = NULL;
  int tensor_bits;
  if (luaT_isudata(L, 2, luaT_checktypename2id(L, "torch.DoubleTensor"))) {
    tensor64 = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.DoubleTensor"));
    tensor_bits = 64;
  } else {
    perror("getFrame: only DoubleTensor is supported");
    lua_pushnil(L);  /* return nil and ... */
    return 1;    
  }

  // Shared Mem Size + dim check
  int buffer_size;
  buffer_size = tensor64->size[tensor64->nDimension-2] * tensor64->size[tensor64->nDimension-1] * 4; // RGB    
  // Check nb of dims
  if (tensor64->nDimension != 2 && tensor64->nDimension != 3) {
    printf("getFrame T64: tensor must have 2 or 3 dimensions\n");
    lua_pushnil(L);
    return 1;
  } 

  // Do that only once
  if (shared_mem_connected == 0) {
    // We use a shared memory segment to make the pixels available to external software
    if ((shmem_key = ftok(path, 'A')) == -1) {
      perror("ftok couldnt get the shared mem descriptor");
      lua_pushnil(L);  /* return nil and ... */
      return 1;
    }
    
    // Sync to the segment
    if((shmem_id = shmget(shmem_key, buffer_size + 16, 0644 | IPC_CREAT)) == -1) {
      perror("shmget couldnt sync the shared mem segment");
      lua_pushnil(L);  /* return nil and ... */
      return 1;
    }

    // and link data to the segment
    buffer = (struct video_buffer *)shmat(shmem_id, (void *)0, 0);

    // Only once !
    printf("buffer connected to shared mem\n");
    shared_mem_connected = 1;
  }
  
  // make a frame request
  buffer->exit = exit;
  if (exit) { 
    // Destroy Shared Mem
    if(shmdt(buffer) == -1) {
      perror("shmdt");
    }
    shared_mem_connected = 0;
    return 0;
  }
  buffer->request_frame = 1;
  while (buffer->request_frame); // request received
  while (!buffer->frame_ready) sleep(0.01); // wait for frame to be ready

  // Fill tensor's storage with shared mem data
  long x, y, k; 
  if (tensor_bits == 64) {
    if (tensor64->nDimension == 2) {  // GREEN Chanel
      for (y=0; y<tensor64->size[0]; y++) {  
        for (x=0; x<tensor64->size[1]; x++) {
          double pix = ((double)buffer->data[(y*buffer->width+x)*buffer->bytes_per_pixel+1]) / 256;
          THDoubleTensor_set2d(tensor64, y, x, pix);
        }
      }
    } else if (tensor64->nDimension == 3) {  // GREEN Chanel
      if (tensor64->size[0] == 1) {
        for (y=0; y<tensor64->size[0]; y++) {
          for (x=0; x<tensor64->size[1]; x++) {
            double pix = ((double)buffer->data[(y*buffer->width+x)*buffer->bytes_per_pixel+1]) / 256; 
            THDoubleTensor_set3d(tensor64, 0, y, x, pix);
          }
        }
      } else if (tensor64->size[0] == 3) {   // RGB Chanels
        for (y=0; y<tensor64->size[1]; y++) {
          for (x=0; x<tensor64->size[2]; x++) {
            for (k=0; k<tensor64->size[0]; k++) {
              double pix = ((double)buffer->data[(y*buffer->width+x)*buffer->bytes_per_pixel+k]) / 256; 
              THDoubleTensor_set3d(tensor64, k, y, x, pix);
            }
          }      
        }
      }
    }
  }
  return 0; 
}

static int l_forkProcess (lua_State *L) {
  const char *exec_cmd = luaL_checkstring(L, 1);
  
  // forget children
  signal(SIGCHLD, SIG_IGN);

  // Fork and exec external process
  pid_t pID = fork();
  if (pID == 0) {
    execlp (exec_cmd, exec_cmd, (char *)0);
  } else if (pID > 0) {
    printf("Started child process with pID %d\n", pID);
  }

  // Return the ret code
  lua_pushnumber(L, (int)pID);
  return 1;
}

static int l_killProcess (lua_State *L) {
  pid_t pID = (pid_t)luaL_checknumber(L, 1);
  printf("Killing process with pID = %d\n", pID);
  kill(pID, SIGINT);
  return 0;
}

static int l_nonBlockingRead (lua_State *L) {
  int flags, flags_saved;
  flags_saved = fcntl(0, F_GETFL, 0);
  flags = flags_saved | O_NONBLOCK;
  fcntl(0, F_SETFL, flags);
  char c = getchar();
  fcntl(0, F_SETFL, flags_saved);
  // return it
  lua_pushnumber(L, c);
  return 1;
}


// Register functions
static const struct luaL_reg camiface [] = {
  {"getSharedFrame", l_getSharedFrame},
  {"forkProcess", l_forkProcess},
  {"killProcess", l_killProcess},
  {"read", l_nonBlockingRead},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libcamiface (lua_State *L) {
  luaL_openlib(L, "libcamiface", camiface, 0);
  return 1; 
}
