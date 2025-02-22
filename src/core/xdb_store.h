/******************************************************************************
* Copyright (c) 2024-present JC Wang. All rights reserved
*
*   https://crossdb.org
*   https://github.com/crossdb-org/crossdb
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
******************************************************************************/

#ifndef __XDB_STORE_H__
#define __XDB_STORE_H__

#define XDB_IDPTR(pStgMgr, id)	((pStgMgr)->pBlkDat1 + id * (pStgMgr)->blk_size)

typedef struct xdb_store_ops {
	int  (*store_open) 	(const char *file, xdb_fd *pFd, xdb_size *pSize);
	int  (*store_close) (xdb_fd *pFd, xdb_size size, void **ptr);
	int  (*store_drop)  (char *file);
	int  (*store_map) 	(xdb_fd fd, xdb_size size, bool bTruncate, void **ptr);
	int  (*store_remap) (xdb_fd fd, xdb_size old_size, xdb_size new_size, void **ptr);
	int  (*store_sync)  (xdb_fd fd, xdb_size size, void *ptr, bool bAsync);
} xdb_store_ops;

#define XDB_ROW_MASK (0x3)

typedef enum {
	XDB_ROW_FREE 	= 0, // 00 ->dirty->trans->commit
	XDB_ROW_DIRTY 	= 1, // 01
	XDB_ROW_COMMIT 	= 2, // 10
	XDB_ROW_TRANS  	= 3, // 11 in trans, not commit
} xdb_rowCtrl_e;

#define XDB_ROW_CTRL(pStgHdr,pRow)	*((uint8_t*)(pRow) + pStgHdr->ctl_off)

#define	XDB_STG_NOALLOC	(1<<0)
#define	XDB_STG_CLEAR	(1<<1)

typedef struct xdb_stghdr_t {
	uint32_t		stg_magic;
	uint32_t		revision;
	uint32_t		blk_size;
	uint32_t		ctl_off;
	uint16_t		blk_off;
	uint8_t			blk_type;
	uint8_t			blk_flags;
	xdb_rowid		blk_head;
	xdb_rowid		blk_tail;
	xdb_rowid		blk_alloc;
	xdb_rowid		blk_maxid;
	xdb_rowid		blk_cap;
	xdb_rowid		blk_limit;
	uint32_t		rsvd2[5];
} xdb_stghdr_t;

typedef struct xdb_stgmgr_t {
	xdb_stghdr_t	*pStgHdr;
	xdb_fd 			stg_fd;
	xdb_store_ops	*pOps;
	void			*pBlkDat;
	void			*pBlkDat1;
	uint32_t		blk_size;
	char			*file;
} xdb_stgmgr_t;

#define XDB_STG_CAP(pStgMgr)	(pStgMgr)->pStgHdr->blk_cap
#define XDB_STG_MAXID(pStgMgr)	(pStgMgr)->pStgHdr->blk_maxid
#define XDB_STG_ALLOC(pStgMgr)	(pStgMgr)->pStgHdr->blk_alloc

#define XDB_ROW_VALID(pTblm,pRow)	((XDB_ROW_CTRL (pTblm->stg_mgr.pStgHdr, pRow) & 0x3) >= XDB_ROW_COMMIT)

XDB_STATIC xdb_rowid 
xdb_stg_alloc (xdb_stgmgr_t *pStgMgr, void **ppRow);

XDB_STATIC int 
xdb_stg_expand (xdb_stgmgr_t *pStgMgr);

XDB_STATIC void 
xdb_stg_free (xdb_stgmgr_t *pStgMgr, xdb_rowid rowid, void *pRow);

#endif // __XDB_STORE_H__
