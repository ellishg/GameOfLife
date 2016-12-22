__kernel void next_generation(__global char * src, __global char * dest, write_only image2d_t framebuffer)
{
  uint gid_x = get_global_id(0);
  uint gid_y = get_global_id(1);

  uint left = (gid_x - 1) % WIDTH;
  uint right = (gid_x + 1) % WIDTH;
  uint up = (gid_y - 1) % HEIGHT;
  uint down = (gid_y + 1) % HEIGHT;

  uint center = gid_x + gid_y * WIDTH;

  uint num_neighbors = src[left + up * WIDTH]
                     + src[gid_x + up * WIDTH]
                     + src[right + up * WIDTH]
                     + src[left + gid_y * WIDTH]
                     + src[right + gid_y * WIDTH]
                     + src[left + down * WIDTH]
                     + src[gid_x + down * WIDTH]
                     + src[right + down * WIDTH];

  // Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
  // Any live cell with two or three live neighbours lives on to the next generation.
  // Any live cell with more than three live neighbours dies, as if by overpopulation.
  // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.

  char new_cell = num_neighbors == 3 || (src[center] && num_neighbors == 2);
  dest[center] = new_cell;

  write_imagef(framebuffer, (int2)(gid_x, gid_y), (float4)(new_cell, new_cell, new_cell, 1));
}
