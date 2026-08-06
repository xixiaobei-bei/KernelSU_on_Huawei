#ifndef _SLAMCOMMON_H
#define _SLAMCOMMON_H

#define MAX_PYRAMID_LAYER          (8) 
#define SLAM_SCOREHIST_NUM (64)
#define SLAM_PATTERN_NUM  (256)

typedef enum _slam_buf_usage_e
{
    BI_SLAM_Y,                       
    BO_SLAM_Y,
    BO_SLAM_STAT,
    BO_SLAM_SCORE_Y,
    SLAM_STREAM_MAX,
}slam_buf_usage_e;

#endif
