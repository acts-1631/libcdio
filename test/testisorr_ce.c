/* Check that a Rock Ridge CE continuation cannot exceed its block. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cdio/iso9660.h>
#include <cdio/rock.h>

iso9660_stat_t *_iso9660_dd_find_lsn(void *image, lsn_t lsn);

iso9660_stat_t *
_iso9660_dd_find_lsn(void *image, lsn_t lsn)
{
  (void) image;
  (void) lsn;
  return NULL;
}

long int
iso9660_iso_seek_read(const iso9660_t *image, void *buffer, lsn_t start,
                      long int blocks)
{
  (void) image;
  (void) start;
  memset(buffer, 0, (size_t) blocks * ISO_BLOCKSIZE);
  return ISO_BLOCKSIZE;
}

void
iso9660_stat_free(iso9660_stat_t *p_stat)
{
  free(p_stat);
}

static void
put_733(uint8_t *field, uint32_t value)
{
  field[0] = value;
  field[1] = value >> 8;
  field[2] = value >> 16;
  field[3] = value >> 24;
  field[4] = value >> 24;
  field[5] = value >> 16;
  field[6] = value >> 8;
  field[7] = value;
}

int
main(void)
{
  uint8_t *record = calloc(1, 62);
  iso9660_stat_t *stat = calloc(1, sizeof(*stat) + 2);
  char name[256] = "";
  int ret = 1;

  if (!record || !stat)
    goto out;

  record[0] = 62;
  record[32] = 1;
  record[33] = 'A';
  record[34] = 'C';
  record[35] = 'E';
  record[36] = 28;
  record[37] = 1;
  put_733(&record[46], ISO_BLOCKSIZE - 1);
  put_733(&record[54], ISO_BLOCKSIZE - 1);
  stat->rr.b3_rock = dunno;

  if (get_rock_ridge_filename((iso9660_dir_t *) record, record, name,
                              stat) != 0)
    goto out;
  if (stat->rr.u_su_fields & ISO_ROCK_SUF_CE)
    goto out;

  ret = 0;
out:
  free(stat);
  free(record);
  return ret;
}
