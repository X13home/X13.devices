/*
Copyright (c) 2011-2020 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions Counter

#include "../config.h"

#if ((defined EXTDIO_USED) && (defined EXTCNT_USED))

static uint16_t cnt_old[EXTCNT_MAXPORT_NR];

/////////////////////////////////////////////////////
// Counter Section ,   depended from extdio.c

static e_MQTTSN_RETURNS_t cntReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
	uint16_t base = pSubidx->Base;
	uint16_t val = hal_cnt_read(base);
	cnt_old[base] = val;
	*pLen = 2;
    pBuf[0] = (uint8_t)(val & 0xFF);
    pBuf[1] = (uint8_t)(val >> 8);
	return MQTTSN_RET_ACCEPTED;
}

static uint8_t cntPollOD(subidx_t * pSubidx)
{
	static uint8_t poll_cnt = 0;
	
    uint16_t base = pSubidx->Base;
	
	if((poll_cnt == 0) && (hal_cnt_read(base) != cnt_old[base]))
	{
		return 1;
	}
		
	poll_cnt--;
    return 0;
}

static e_MQTTSN_RETURNS_t cntWriteOD(subidx_t * pSubidx, uint8_t len  __attribute__ ((unused)), uint8_t *pBuf)
{
    uint16_t base = pSubidx->Base;
	uint16_t value = pBuf[1];
	value <<= 8;
	value |= pBuf[0];
	
	cnt_old[base] = value;
	hal_cnt_write(base, value);

    return MQTTSN_RET_ACCEPTED;
}

bool cntCheckSubidx(subidx_t * pSubidx)
{
    eObjTyp_t type = pSubidx->Type;
    uint16_t base = pSubidx->Base;
    if(((type != objPinNPN) && (type != objPinPNP)) ||
        (hal_dio_base2pin(base) == 0xFF) ||
        (hal_cnt_checkbase(base) != true))
        return false;

    return true;
}

e_MQTTSN_RETURNS_t cntRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;
    uint8_t dio = hal_dio_base2pin(base);
    
    if((dioCheckBase(dio) != 0) || hal_cnt_busy(base))
        return MQTTSN_RET_REJ_INV_ID;

    dioTake(dio);
    hal_cnt_configure(base, (pIdx->sidx.Type == objPinNPN));

	cnt_old[base] = 0;

	pIdx->cbRead  = &cntReadOD;
	pIdx->cbPoll  = &cntPollOD;
    pIdx->cbWrite = &cntWriteOD;
	
    return MQTTSN_RET_ACCEPTED;
}

void cntDeleteOD(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;

    hal_cnt_delete(base);
    dioRelease(hal_dio_base2pin(base));
}

#endif	// EXTDIO_USED & EXTCNT_USED