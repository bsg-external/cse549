### Results for Group_2 (Dec. 15)

#### Speedup over previous leader Geomean: 386,241/454,584 = 0.85X compared with group_3

Geometric mean of the runtime: 454,584 Cycles
| Sorting Size | Cycle Count | Core Utilization |
|--------------|-------------|------------------|
| 2^12         | 288594      | 20.34%           |
| 2^13         | 463638      | 21.27%           |
| 2^14         | 702063      | 16.16%           |


#### a. TILE_GROUP_DIM_X = 4 and TILE_GROUP_DIM_Y = 2
| Sorting Size | Cycle Count | Core Utilization |
|--------------|-------------|------------------|
| 2^12         | 533041      | 31.42%           |
| 2^13         | 1048470     | 31.21%           |
| 2^14         | 2078281     | 31.12%           |

#### b. TILE_GROUP_DIM_X = 4 and TILE_GROUP_DIM_Y = 4
| Sorting Size | Cycle Count | Core Utilization |
|--------------|-------------|------------------|
| 2^12         | 358754      | 25.43%           |
| 2^13         | 674839      | 25.36%           |
| 2^14         | 1328216     | 24.91%           |

#### c. TILE_GROUP_DIM_X = 8 and TILE_GROUP_DIM_Y = 4
| Sorting Size | Cycle Count | Core Utilization |
|--------------|-------------|------------------|
| 2^12         | 288594      | 20.34%           |
| 2^13         | 463638      | 21.27%           |
| 2^14         | 824053      | 21.66%           |

#### d. TILE_GROUP_DIM_X = 8 and TILE_GROUP_DIM_Y = 8
| Sorting Size | Cycle Count | Core Utilization |
|--------------|-------------|------------------|
| 2^12         | 356443      | 15.02%           |
| 2^13         | 490004      | 15.00%           |
| 2^14         | 702063      | 16.16%           |

#### e. TILE_GROUP_DIM_X = 16 and TILE_GROUP_DIM_Y = 8
| Sorting Size | Cycle Count | Core Utilization |
|--------------|-------------|------------------|
| 2^12         | 574754      | 12.74%           |
| 2^13         | 633021      | 21.27%           |
| 2^14         | 734454      | 14.05%           |
