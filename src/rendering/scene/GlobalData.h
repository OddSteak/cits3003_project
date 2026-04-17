#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include "rendering/cameras/CameraInterface.h"

/// A base class for all GlobalData structs which contain camera data,
/// to provide a common interface for applying a camera state.
class GlobalDataCameraInterface {
public:
    virtual void use_camera(const CameraInterface& camera_interface) = 0;
};


#endif //GLOBAL_DATA_H
