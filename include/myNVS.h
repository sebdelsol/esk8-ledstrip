#pragma once

extern "C" 
{
  #include "esp_partition.h"
  #include "esp_err.h"
  #include "nvs_flash.h"
  #include "nvs.h"
}

class MyNvs 
{
  nvs_handle  _nvs_handle;    
  bool        mIsOK = false;

  bool commit()
  {
    esp_err_t err = nvs_commit(_nvs_handle);
    return err == ESP_OK;
  };

public:
  bool isOK() { return mIsOK; };

  bool begin(const char* namespaceNvs)
  {
    _log << "NVS begin";
    #define TASKTEST(tst, task, failed, ok) _log << "..." << task << "..." << (tst ? failed : ok); if(tst)

    esp_err_t err = nvs_flash_init();
    TASKTEST(err != ESP_OK, "init", "failed", "done")
    {
      TASKTEST(err == ESP_ERR_NVS_NO_FREE_PAGES, "is free page", "no\n", "yes") return false;

      // erase and reinit
      const esp_partition_t *nvs_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
      TASKTEST(nvs_partition == NULL, "find partion", "failed\n", "found") return false;
      
      _log << "NVS reformat...";
      err = esp_partition_erase_range(nvs_partition, 0, nvs_partition->size);
      TASKTEST(err != ESP_OK, "reformat", "failed\n" ,"done") return false;

      err = nvs_flash_init();
      TASKTEST(err != ESP_OK, "reinit", "failed\n" , "done") return false;
    }

    err = nvs_open(namespaceNvs, NVS_READWRITE, &_nvs_handle);
    TASKTEST(err != ESP_OK, "open", "failed\n" , "done\n") return false;

    mIsOK = true;
    return true;
  };

  bool setuint32(const char* key, uint32_t value)
  {
    esp_err_t err = nvs_set_u32(_nvs_handle, key, value);
    return err == ESP_OK ? commit() : false;
  };

  bool getuint32(const char* key, uint32_t& value)
  {
    esp_err_t err = nvs_get_u32(_nvs_handle, key, &value);
    return err == ESP_OK;
  };
};
