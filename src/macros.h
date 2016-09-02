#define SHUFFLE_ITERATION(iteration)\
      if (v == salt_length) {\
      v = 0;\
      }\
      p += salt[v];\
      j = (salt[v] + v + p) % iteration;\
      temp = str[iteration];\
      str[iteration] = str[j];\
      str[j] = temp;\
      --i;\
      ++v;
