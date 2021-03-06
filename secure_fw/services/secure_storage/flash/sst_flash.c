/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "sst_flash.h"

#include <string.h>
#include "cmsis_compiler.h"
#include "Driver_Flash.h"
#include "secure_fw/services/secure_storage/sst_utils.h"
#include "tfm_sst_defs.h"

#ifndef SST_FLASH_AREA_ADDR
#error "SST_FLASH_AREA_ADDR must be defined in flash_layout.h file"
#endif

#ifndef FLASH_DEV_NAME
#error "FLASH_DEV_NAME must be defined in flash_layout.h file"
#endif

/* Import the CMSIS flash device driver */
extern ARM_DRIVER_FLASH FLASH_DEV_NAME;

#define BLOCK_START_OFFSET  0

/*
 * \brief Gets physical address of the given block ID.
 *
 * \param[in]  block_id  Block ID
 * \param[in]  offset    Offset position from the init of the block
 *
 * \returns Returns physical address for the given block ID.
 */
__attribute__((always_inline))
__STATIC_INLINE uint32_t get_phys_address(uint32_t block_id, uint32_t offset)
{
    return (SST_FLASH_AREA_ADDR + (block_id * SST_BLOCK_SIZE) + offset);
}

static enum psa_sst_err_t flash_read(uint32_t flash_addr, uint8_t *buff,
                                     uint32_t size)
{
    int32_t err;

    err = FLASH_DEV_NAME.ReadData(flash_addr, buff, size);
    if (err != ARM_DRIVER_OK) {
        return PSA_SST_ERR_SYSTEM_ERROR;
    }

    return PSA_SST_ERR_SUCCESS;
}

static enum psa_sst_err_t flash_write(uint32_t flash_addr, const uint8_t *buff,
                                      uint32_t size)
{
    int32_t err;

    err = FLASH_DEV_NAME.ProgramData(flash_addr, buff, size);
    if (err != ARM_DRIVER_OK) {
        return PSA_SST_ERR_SYSTEM_ERROR;
    }

    return PSA_SST_ERR_SUCCESS;
}

static enum psa_sst_err_t flash_erase(uint32_t flash_addr)
{
    int32_t err;

    err = FLASH_DEV_NAME.EraseSector(flash_addr);
    if (err != ARM_DRIVER_OK) {
        return PSA_SST_ERR_SYSTEM_ERROR;
    }

    return PSA_SST_ERR_SUCCESS;
}

enum psa_sst_err_t sst_flash_read(uint32_t block_id, uint8_t *buff,
                                  uint32_t offset, uint32_t size)
{
    uint32_t flash_addr;

    /* Gets flash address location defined by block ID and offset
     * parameters.
     */
    flash_addr = get_phys_address(block_id, offset);

    return flash_read(flash_addr, buff, size);
}

enum psa_sst_err_t sst_flash_write(uint32_t block_id, const uint8_t *buff,
                                   uint32_t offset, uint32_t size)
{
    uint32_t flash_addr;

    /* Gets flash address location defined by block ID and offset
     * parameters.
     */
    flash_addr = get_phys_address(block_id, offset);

    return flash_write(flash_addr, buff, size);
}

enum psa_sst_err_t sst_flash_block_to_block_move(uint32_t dst_block,
                                                 uint32_t dst_offset,
                                                 uint32_t src_block,
                                                 uint32_t src_offset,
                                                 uint32_t size)
{
    static uint8_t dst_block_data_copy[SST_BLOCK_SIZE];
    enum psa_sst_err_t err;
    uint32_t dst_flash_addr;
    uint32_t src_flash_addr;

    /* Gets flash address location defined by block ID and offset
     * parameters.
     */
    src_flash_addr = get_phys_address(src_block, src_offset);

    /* Reads data from source block and store it in the in-memory copy of
     * destination content.
     */
    err = flash_read(src_flash_addr, dst_block_data_copy, size);
    if (err != PSA_SST_ERR_SUCCESS) {
        return err;
    }

    /* Gets flash address location defined by block ID and offset
     * parameters.
     */
    dst_flash_addr = get_phys_address(dst_block, dst_offset);

    /* Writes in flash the in-memory block content after modification */
    err = flash_write(dst_flash_addr, dst_block_data_copy, size);

    return err;
}

enum psa_sst_err_t sst_flash_erase_block(uint32_t block_id)
{
    uint32_t flash_addr;

    /* Calculate flash address location defined by block ID and
     * BLOCK_START_OFFSET parameters.
     */
    flash_addr = get_phys_address(block_id, BLOCK_START_OFFSET);

    return flash_erase(flash_addr);
}
