#ifndef __MAPGEN_H__
#define __MAPGEN_H__

struct map_s;

bool mapgen_space_for_tree(struct map_s *map, int x, int y, int z, int height);
void mapgen_grow_tree(struct map_s *map, int x, int y, int z, int height);

void mapgen_flat_generate(struct map_s *map);
void mapgen_debug_generate(struct map_s *map);
void mapgen_classic_generate(struct map_s *map);

#endif