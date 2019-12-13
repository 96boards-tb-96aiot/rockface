/****************************************************************************
*
*    Copyright (c) 2017 - 2019 by Rockchip Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Rockchip Corporation. This is proprietary information owned by
*    Rockchip Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Rockchip Corporation.
*
*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <sys/time.h>
#include <stdlib.h>

#include "rockface.h"

int main(int argc, char** argv) {

    rockface_ret_t ret;
    struct timeval tv;

    if( argc != 3 ){
        printf("\nUsage: face_analyze <licence file> <path_to_image>\n");
        return -1;
    }

    const char *licence_path = argv[1];

    // read image
    const char *img_path = argv[2];

    /*************** Creat Handle ***************/
    rockface_handle_t face_handle = rockface_create_handle();

    ret = rockface_set_licence(face_handle, licence_path);
    if (ret < 0) {
        printf("Error: authorization error %d!", ret);
        return ret;
    }

    ret = rockface_init_detector(face_handle);
    ret = rockface_init_analyzer(face_handle);

    // read image
    rockface_image_t input_image;
    rockface_image_read(img_path, &input_image, 1);

    // create rockface_face_array_t for store result
    rockface_det_array_t face_array;
    memset(&face_array, 0, sizeof(rockface_det_array_t));

    // detect face
    ret = rockface_detect(face_handle, &input_image, &face_array);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("rockface_face_detect error %d\n", ret);
        return -1;
    }

    rockface_det_array_t tracked_face_array;
    rockface_track(face_handle, &input_image, 1, &face_array, &tracked_face_array);
    rockface_track(face_handle, &input_image, 1, &face_array, &tracked_face_array);

    for (int i = 0; i < tracked_face_array.count; i++) {
        rockface_det_t *det_face = &(tracked_face_array.face[i]);
        // face align
        rockface_image_t aligned_img;
        memset(&aligned_img, 0, sizeof(rockface_image_t));
        ret = rockface_align(face_handle, &input_image, &(det_face->box), NULL, &aligned_img);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("error align face %d\n", ret);
            return -1;
        }
        // face attribute
        rockface_attribute_t face_attr;
        ret = rockface_attribute(face_handle, &aligned_img, &face_attr);
        
        rockface_image_release(&aligned_img);

        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("error rockface_attribute %d\n", ret);
            return -1;
        }
        // face landmark
        rockface_landmark_t face_landmark;
        ret = rockface_landmark5(face_handle, &input_image, &(det_face->box), &face_landmark);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("error rockface_landmarke %d\n", ret);
            return -1;
        }
        // face angle
        rockface_angle_t face_angle;
        ret = rockface_angle(face_handle, &face_landmark, &face_angle);
        printf("%d %d box=(%d %d %d %d) score=%f landmark_score=%f age=%d gender=%d angle=(%f %f %f)\n",
            i, det_face->id, det_face->box.left, det_face->box.top, det_face->box.right, det_face->box.bottom, det_face->score, face_landmark.score,
            face_attr.age, face_attr.gender,
            face_angle.pitch, face_angle.roll, face_angle.yaw);
    }


    // release image
    rockface_image_release(&input_image);

    //release handle
    rockface_release_handle(face_handle);
    return 0;
}