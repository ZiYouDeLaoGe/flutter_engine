// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_CONTEXT_GL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_CONTEXT_GL_H_

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/platform/android/android_environment_gl.h"
#include "flutter/shell/platform/android/context/android_context.h"
#include "flutter/shell/platform/android/surface/android_native_window.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flutter {

//------------------------------------------------------------------------------
/// Holds an `EGLSurface` reference.
///
///
/// This can be used in conjunction to unique_ptr to provide better guarantees
/// about the lifespan of the `EGLSurface` object.
///
class AndroidEGLSurfaceDamage;

/// Result of calling MakeCurrent on AndroidEGLSurface.
enum class AndroidEGLSurfaceMakeCurrentStatus {
  /// Success, the egl context for the surface was already current.
  kSuccessAlreadyCurrent,
  /// Success, the egl context for the surface made current.
  kSuccessMadeCurrent,
  /// Failed to make the egl context for the surface current.
  kFailure,
};

class AndroidEGLSurface {
 public:
  AndroidEGLSurface(EGLSurface surface, EGLDisplay display, EGLContext context);
  ~AndroidEGLSurface();

  //----------------------------------------------------------------------------
  /// @return     Whether the current `EGLSurface` reference is valid. That is,
  /// if
  ///             the surface doesn't point to `EGL_NO_SURFACE`.
  ///
  bool IsValid() const;

  //----------------------------------------------------------------------------
  /// @brief      Binds the EGLContext context to the current rendering thread
  ///             and to the draw and read surface.
  ///
  /// @return     Whether the surface was made current.
  ///
  AndroidEGLSurfaceMakeCurrentStatus MakeCurrent() const;

  //----------------------------------------------------------------------------
  ///
  /// @return     Whether target surface supports partial repaint.
  ///
  bool SupportsPartialRepaint() const;

  //----------------------------------------------------------------------------
  /// @brief      This is the minimal area that needs to be repainted to get
  ///             correct result.
  ///
  /// With double or triple buffering this buffer content may lag behind
  /// current front buffer and the rect accounts for accumulated damage.
  ///
  /// @return     The area of current surface where it is behind front buffer.
  ///
  std::optional<SkIRect> InitialDamage();

  //----------------------------------------------------------------------------
  /// @brief      Sets the damage region for current surface. Corresponds to
  //              eglSetDamageRegionKHR
  void SetDamageRegion(const std::optional<SkIRect>& buffer_damage);

  //----------------------------------------------------------------------------
  /// @brief      This only applies to on-screen surfaces such as those created
  ///             by `AndroidContextGL::CreateOnscreenSurface`.
  ///
  /// @return     Whether the EGL surface color buffer was swapped.
  ///
  bool SwapBuffers(const std::optional<SkIRect>& surface_damage);

  //----------------------------------------------------------------------------
  /// @return     The size of an `EGLSurface`.
  ///
  SkISize GetSize() const;

 private:
  /// Returns true if the EGLContext held is current for the display and surface
  bool IsContextCurrent() const;

  const EGLSurface surface_;
  const EGLDisplay display_;
  const EGLContext context_;
  std::unique_ptr<AndroidEGLSurfaceDamage> damage_;
};

//------------------------------------------------------------------------------
/// The Android context is used by `AndroidSurfaceGL` to create and manage
/// EGL surfaces.
///
/// This context binds `EGLContext` to the current rendering thread and to the
/// draw and read `EGLSurface`s.
///
class AndroidContextGL : public AndroidContext {
 public:
  AndroidContextGL(AndroidRenderingAPI rendering_api,
                   fml::RefPtr<AndroidEnvironmentGL> environment,
                   const TaskRunners& taskRunners,
                   uint8_t msaa_samples);

  ~AndroidContextGL();

  //----------------------------------------------------------------------------
  /// @brief      Allocates an new EGL window surface that is used for on-screen
  ///             pixels.
  ///
  /// @return     The window surface.
  ///
  std::unique_ptr<AndroidEGLSurface> CreateOnscreenSurface(
      fml::RefPtr<AndroidNativeWindow> window) const;

  //----------------------------------------------------------------------------
  /// @brief      Allocates an 1x1 pbuffer surface that is used for making the
  ///             offscreen current for texture uploads.
  ///
  /// @return     The pbuffer surface.
  ///
  std::unique_ptr<AndroidEGLSurface> CreateOffscreenSurface() const;

  //----------------------------------------------------------------------------
  /// @brief      Allocates an 1x1 pbuffer surface that is used for making the
  ///             onscreen context current for snapshotting.
  ///
  /// @return     The pbuffer surface.
  ///
  std::unique_ptr<AndroidEGLSurface> CreatePbufferSurface() const;

  //----------------------------------------------------------------------------
  /// @return     The Android environment that contains a reference to the
  /// display.
  ///
  fml::RefPtr<AndroidEnvironmentGL> Environment() const;

  //----------------------------------------------------------------------------
  /// @return     Whether the current context is valid. That is, if the EGL
  ///             contexts were successfully created.
  ///
  bool IsValid() const override;

  //----------------------------------------------------------------------------
  /// @return     Whether the current context was successfully clear.
  ///
  bool ClearCurrent() const;

  //----------------------------------------------------------------------------
  /// @brief      Create a new EGLContext using the same EGLConfig.
  ///
  /// @return     The EGLContext.
  ///
  EGLContext CreateNewContext() const;

  //----------------------------------------------------------------------------
  /// @brief      The EGLConfig for this context.
  ///
  EGLConfig Config() const { return config_; }

 private:
  fml::RefPtr<AndroidEnvironmentGL> environment_;
  EGLConfig config_;
  EGLContext context_;
  EGLContext resource_context_;
  bool valid_ = false;
  TaskRunners task_runners_;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidContextGL);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_CONTEXT_GL_H_
