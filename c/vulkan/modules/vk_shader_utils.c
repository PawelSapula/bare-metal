#include "utils.h"
#include "vk_shader_utils.h"
#include <stdio.h>

static struct vk_utils_info_s info;

void source_shader_utils(struct vk_utils_info_s info_s){
  info = info_s;
}

static int readShaderFile(const char* path, size_t* p_size, unsigned char* buf) {
  FILE *pfile;
  pfile = fopen(path, "rb");

  if(pfile == NULL) {
    printf("Unable to get specified shader file.");
    return FAILURE;
  }

  fseek(pfile, 0, SEEK_END);
  *p_size = ftell(pfile);
  rewind(pfile);

  if (buf != NULL) {
    fread((void*)buf, *p_size, 1, pfile);
  }

  fclose(pfile);
  return SUCCESS;
}

VkShaderModule create_shader_module(const char* path) {
  
  size_t shader_size = 0;
  int result = readShaderFile(path, &shader_size, NULL);
  if(result == FAILURE) {
    return NULL;
  }
  unsigned char* shader = GET_ARRAY(unsigned char, shader_size);

  readShaderFile(path, &shader_size, shader);

  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_size;
  create_info.pCode = (const uint32_t*)shader; // Oops might go wrong

  VkShaderModule shader_module;
  VkResult res = vkCreateShaderModule(info.device, &create_info, NULL, &shader_module);
  free(shader);
  if(res != VK_SUCCESS) {
    printf("Shader module creation went wrong! Code: %d\n", res);
    return NULL;
  }
  return shader_module;
}
