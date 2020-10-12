#pragma once
#include <log.h>

extern "C" 
{
  #include "esp_partition.h"
  #include "esp_err.h"
  #include "nvs_flash.h"
  #include "nvs.h"
}

class MyNvs 
{
  nvs_handle  mHandle;    
  bool        mIsOK = false;

  bool commit()
  {
    esp_err_t err = nvs_commit(mHandle);
    return err == ESP_OK;
  };

public:
  bool isOK() { return mIsOK; };

  bool begin(const char* namespaceNvs)
  {
    _log << "NVS";
    #define TASKTEST(tst, task, failed, ok) _log << "..." << task << "..." << (tst ? failed : ok); if(tst)

    esp_err_t err = nvs_flash_init();
    TASKTEST(err != ESP_OK, "init", "failed", "done")
    {
      TASKTEST(err == ESP_ERR_NVS_NO_FREE_PAGES, "is free page", "no", "yes") return false;

      // erase and reinit
      const esp_partition_t *nvs_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
      TASKTEST(nvs_partition == NULL, "find partion", "failed", "found") return false;
      
      _log << "NVS reformat...";
      err = esp_partition_erase_range(nvs_partition, 0, nvs_partition->size);
      TASKTEST(err != ESP_OK, "reformat", "failed" ,"done") return false;

      err = nvs_flash_init();
      TASKTEST(err != ESP_OK, "reinit", "failed" , "done") return false;
    }

    err = nvs_open(namespaceNvs, NVS_READWRITE, &mHandle);
    TASKTEST(err != ESP_OK, "open", "failed" , "done") return false;

    mIsOK = true;
    return true;
  };

  bool setuint(const char* key, uint32_t value)
  {
    assert(key != nullptr);
    esp_err_t err = nvs_set_u32(mHandle, key, value);
    return err == ESP_OK ? commit() : false;
  };

  bool getuint(const char* key, uint32_t& value)
  {
    assert(key != nullptr);
    esp_err_t err = nvs_get_u32(mHandle, key, &value);
    return err == ESP_OK;
  };

  bool erase(const char* key)
  {
    assert(key != nullptr);
    esp_err_t err = nvs_erase_key(mHandle, key);
    return err == ESP_OK ? commit() : false;
  };

};
