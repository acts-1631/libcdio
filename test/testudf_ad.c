/* Check that UDF extent walking rejects a descriptor beyond the allocation
   descriptor area. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#define __CDIO_CONFIG_H__ 1
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/udf/udf_private.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

int
main(void)
{
  struct {
    udf_dirent_t dirent;
    union {
      udf_short_ad_t short_ad;
      udf_long_ad_t long_ad;
    } next_ad;
  } *test = calloc(1, sizeof(*test));
  udf_t udf = {0};
  uint8_t buffer[UDF_BLOCKSIZE];
  udf_short_ad_t *ad;
  ssize_t nread;
  int ret = 1;

  if (!test)
    return ret;

  udf.b_stream = true;
  udf.stream = cdio_stdio_new(DATA_DIR "/cdda.bin");
  if (!udf.stream)
    goto out;

  test->dirent.p_udf = &udf;
  test->dirent.fe.icb_tag.strat_type = ICBTAG_STRATEGY_TYPE_4;
  test->dirent.fe.icb_tag.flags = ICBTAG_FLAG_AD_SHORT;
  test->dirent.fe.info_len = 4096;
  test->dirent.fe.u_extended_attr = 1864;
  test->dirent.fe.u_alloc_descs = sizeof(udf_short_ad_t);

  ad = (udf_short_ad_t *)&test->dirent.fe.u.pad_to_one_block[1864];
  ad->len = UDF_BLOCKSIZE;
  ad->pos = 0;

  /* This descriptor is just beyond the file entry. The unfixed extent walk
     would read it as a second extent. */
  test->next_ad.short_ad.len = 2 * UDF_BLOCKSIZE;
  test->next_ad.short_ad.pos = 0;

  nread = udf_read_block(&test->dirent, buffer, 1);
  if (nread != UDF_BLOCKSIZE)
    goto close;

  nread = udf_read_block(&test->dirent, buffer, 1);
  if (nread != DRIVER_OP_ERROR)
    goto close;

  memset(&test->dirent.fe, 0, sizeof(test->dirent.fe));
  udf.i_position = 0;
  test->dirent.fe.icb_tag.strat_type = ICBTAG_STRATEGY_TYPE_4;
  test->dirent.fe.icb_tag.flags = ICBTAG_FLAG_AD_LONG;
  test->dirent.fe.info_len = 4096;
  test->dirent.fe.u_extended_attr = 1856;
  test->dirent.fe.u_alloc_descs = sizeof(udf_long_ad_t);

  {
    udf_long_ad_t *long_ad =
      (udf_long_ad_t *)&test->dirent.fe.u.pad_to_one_block[1856];
    long_ad->len = UDF_BLOCKSIZE;
    long_ad->loc.lba = 0;
  }
  test->next_ad.long_ad.len = 2 * UDF_BLOCKSIZE;
  test->next_ad.long_ad.loc.lba = 0;

  nread = udf_read_block(&test->dirent, buffer, 1);
  if (nread != UDF_BLOCKSIZE)
    goto close;

  nread = udf_read_block(&test->dirent, buffer, 1);
  if (nread != DRIVER_OP_ERROR)
    goto close;

  ret = 0;
close:
  cdio_stdio_destroy(udf.stream);
out:
  free(test);
  return ret;
}
