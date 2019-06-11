# Paolo:
# the problem is that onPictureTaken is not called as callback !
# So I have no error but the image is not saved.

# https://groups.google.com/forum/#!topic/kivy-users/hiyAJqjhFjo

'''The raw callback isn't supported on all devices/hardware, so don't
get surprised it you get it with data=None, and/or out of order.

I've stripped down some of my code to make it simpler and am attaching
a short example for a camera preview class. Not perfect, but I hope it
helps.  My code has two possible handlers for the JPEG callback,m one
for simply saving it to file, the other for loading it into a Texture
- I currently use the first one but left the other one in just in
case.

Please note, my example relies on some way of getting a surface holder
from the PythonActivity instance in order to do the preview. In my
case, I've patched PythonActivity.java as follows:

    PythonActivity class should implement SurfaceHolder.Callback
    (implements SurfaceHolder.Callback) In onCreate(), call this
    function before creating the SDLSurfaceView instance (make sure
    you add the required member variables to the PythonActivity
    class):

    protected void onCreateBeforeSDLSurface() {
            mFrameLayout = new FrameLayout(this);
            mCameraView = new SurfaceView(this);
            mCameraView.setZOrderOnTop(false);
            mCameraView.setFocusable(false);
            mCameraSurfaceHolder = mCameraView.getHolder();
            mCameraSurfaceHolder.addCallback(this);
            mCameraSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
            mFrameLayout.addView(mCameraView);
            mCameraView.setVisibility(View.VISIBLE);
        } 

 

    Call this function instead of setContentView(mView), pass mView to the function:

        protected void onCreateAfterSurface(SurfaceView mView) {
            mView.setZOrderOnTop(true);    // necessary
            mView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
            mFrameLayout.addView(mView);
            setContentView(mFrameLayout);
        }

    Implement the SurfaceHolder.Callback interface:

        public void surfaceCreated(SurfaceHolder holder) {
            mCameraSurfaceReady = true;
        }

        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        }

        public void surfaceDestroyed(SurfaceHolder holder) {
            mCameraSurfaceReady = false;
        }

    Add this function: 

        public SurfaceHolder getCameraSurfaceHolder() {
            if (mCameraSurfaceReady)
                return mCameraSurfaceHolder;
            return null;
        }


Hope it helps :)

Let me know if you have any issues or if I missed anything.

'''

from kivy.properties import BooleanProperty, ListProperty, NumericProperty, ObjectProperty, StringProperty
from kivy.logger import Logger
from kivy.utils import platform, boundary
from kivy.core.audio import SoundLoader
from kivy.clock import Clock
from kivy.graphics.texture import Texture
from kivy.uix.widget import Widget

from functools import partial
import operator
from os.path import join

from jnius import autoclass, cast, PythonJavaClass, java_method

AndroidCamera = autoclass('android.hardware.Camera')
CameraInfo = autoclass('android.hardware.Camera$CameraInfo')
CameraArea = autoclass('android.hardware.Camera$Area')
Surface = autoclass('android.view.Surface')
File = autoclass('java.io.File')
FileOutputStream = autoclass('java.io.FileOutputStream')
Rect = autoclass('android.graphics.Rect')
ArrayList = autoclass('java.util.ArrayList')
Parameters = autoclass('android.hardware.Camera$Parameters')
PythonActivity = autoclass('org.renpy.android.PythonActivity')
theActivity = cast('org.renpy.android.PythonActivity', PythonActivity.mActivity)

# Make sure the user controls the volume of the audio stream we're actually using
AudioManager = autoclass('android.media.AudioManager')
theActivity.setVolumeControlStream(AudioManager.STREAM_MUSIC)

BuildVERSION = autoclass('android.os.Build$VERSION')
api_version = int(BuildVERSION.SDK_INT)


def map_List(func, l):
	res = []
	if l is not None:
		it = l.iterator()
		while it.hasNext():
			res.append(func(it.next()))
	return res
	

class _CameraFocusMoveCallback(PythonJavaClass):
	__javainterfaces__ = ['android.hardware.Camera$AutoFocusMoveCallback']
	__javacontext__ = 'app'

	def __init__(self, callback):
		self.callback = callback
		super(_CameraFocusMoveCallback, self).__init__()

	@java_method('(ZLandroid/hardware/Camera;)V')
	def onAutoFocusMoving(self, start, camera):
		self.callback(start)

		
class _CameraFocusCallback(PythonJavaClass):
	__javainterfaces__ = ['android.hardware.Camera$AutoFocusCallback']
	__javacontext__ = 'app'

	def __init__(self, callback):
		self.callback = callback
		super(_CameraFocusCallback, self).__init__()

	@java_method('(ZLandroid/hardware/Camera;)V')
	def onAutoFocus(self, success, camera):
		self.callback(success)

		
class _CameraShutterCallback(PythonJavaClass):
	__javainterfaces__ = ['android.hardware.Camera$ShutterCallback']
	__javacontext__ = 'app'

	def __init__(self, callback):
		self.callback = callback
		super(_CameraShutterCallback, self).__init__()

	@java_method('()V')
	def onShutter(self):
		self.callback()

		
class _CameraPictureCallback(PythonJavaClass):
	__javainterfaces__ = ['android.hardware.Camera$PictureCallback']
	__javacontext__ = 'app'

	def __init__(self, callback):
		self.callback = callback
		super(_CameraPictureCallback, self).__init__()

	@java_method('([BLandroid/hardware/Camera;)V')
	def onPictureTaken(self, data, camera):
		self.callback(data)

		
class CameraPreview(Widget):
	__events__ = ('on_capture_completed', 'on_focus_completed')

	camera_id = NumericProperty(0)
	play = BooleanProperty(False)

	preview_size = ListProperty()
	picture_size = ListProperty()
	rotation = NumericProperty(0)

	# Try to get max preview/picture size if no preference is provided
	prefer_max_size_if_not_specified = BooleanProperty(True)
	preferred_preview_size = ListProperty([])
	preferred_picture_size = ListProperty([])

	supported_picture_sizes = ListProperty([])
	supported_preview_sizes = ListProperty([])
	#supported_flash_modes = ListProperty([])
	supported_focus_modes = ListProperty([])

	camera = ObjectProperty()
	params = ObjectProperty()
	info = ObjectProperty()

	focus_areas_supported = BooleanProperty(False)
	metering_areas_supported = BooleanProperty(False)
	autofocus_supported = BooleanProperty(False)
	default_focus_weight = NumericProperty(1000)
	# size factor for calculating metering area size from focus area size
	metering_area_factor = NumericProperty(2.5)
	focus_move_sound = None
	focus_done_sound = None
	shutter_sound = None

	captured_size = None
	captured_data = None
	captured_filename = StringProperty()
	captured_texture = ObjectProperty()

	focus_rect_size = ListProperty([50, 50])

	_rotation_to_degrees = {
		Surface.ROTATION_0: 0,
		Surface.ROTATION_90: 90,
		Surface.ROTATION_180: 180,
		Surface.ROTATION_270: 270
	}

	_flash_mode_to_android = {
		'off': Parameters.FLASH_MODE_OFF,
		'on': Parameters.FLASH_MODE_ON,
		'auto': Parameters.FLASH_MODE_AUTO,
		'red eye': Parameters.FLASH_MODE_RED_EYE,
		'torch': Parameters.FLASH_MODE_TORCH
	}

	def __init__(self, **kwargs):
		super(CameraPreview, self).__init__(**kwargs)
		self.bind(
			preferred_picture_size=self.resolve_picture_size,
			supported_picture_sizes=self.resolve_picture_size,
			preferred_preview_size=self.resolve_preview_size,
			supported_preview_sizes=self.resolve_preview_size,
			autofocus_supported=self.update_focus_feature,
			focus_areas_supported=self.update_focus_feature)
		self.focus_move_sound = SoundLoader.load('sounds/camera-focusing.mp3')
		self.focus_done_sound = SoundLoader.load('sounds/camera-focus-beep.mp3')
		self.shutter_sound = SoundLoader.load('sounds/camera-shutter-click.mp3')

	def update_focus_feature(self, *args):
		self.focus_supported = self.autofocus_supported and self.focus_areas_supported

	def _rect_to_relative(self, center, rsize, wsize):
		''' Translate a rect defined by (possibly rotated) pixel center (x,y) and size (w,h) into
			[0,1] range left,top,right,bottom relating to rotation=0
		'''
		sw, sh = map(float, wsize)
		x, y = map(float, center)
		w, h = map(float, rsize)
		# translate from openGL space to android, [0,1] range
		b = partial(boundary, minvalue=0., maxvalue=1.)
		left, top = map(b, [(x - w / 2) / sw, (sh - (y + h / 2)) / sh])
		w, h = map(b, [w / sw, h / sh])
		right, bottom = left + w, top + h
		# transpose if rotated
		if self.rotation in [90]:
			left, top, right, bottom = top, 1 - right, bottom, 1 - left
		elif self.rotation in [180]:
			left, top, right, bottom = 1 - right, 1 - bottom, 1 - left, 1 - top
		elif self.rotation in [270]:
			left, top, right, bottom = 1 - bottom, left, 1 - top, right
		return left, top, right, bottom

	def save_jpeg(self, data):
		''' Save data from the camera's JPEG callback into a file '''
		filename = 'sample.jpg'

                Logger.info('CameraPreview: trying to save jpeg file (%s)' % filename)

		try:
			with open(filename, 'wb') as f:
				f.write(data.tostring())
			self.captured_filename = filename
			self.captured_size = self.picture_size
			if self.captured_size[0] > self.captured_size[1]:
				self.captured_size = [ self.picture_size[1], self.picture_size[0] ]
			# Dispatch captured event from the event loop, not from this callback's context

			Logger.info('CameraPreview: saving jpeg file (%s)' % self.captured_filename)

			Clock.schedule_once(self.finish_save_jpeg, 0)
		except Exception as e:
			Logger.error('CameraPreview: failed saving jpeg file (%s)' % e)

	def finish_save_jpeg(self, *args):
		self.dispatch('on_capture_completed', self.captured_filename, self.captured_size)

	def on_capture_completed(self, filename_or_texture, size):
                Logger.info('CameraPreview: saved jpeg file %s size %s' % (filename_or_texture,str(size)))
                
		
	def load_jpeg_to_tex_data(self, data):
		''' Load data from the camera's JPEG callback into a kivy texture '''
		Bitmap = autoclass('android.graphics.Bitmap')
		BitmapFactory = autoclass('android.graphics.BitmapFactory')
		Matrix = autoclass('android.graphics.Matrix')
		ByteBuffer = autoclass('java.nio.ByteBuffer')
		bitmap = BitmapFactory.decodeByteArray(data, 0, len(data), None)
		matrix = Matrix()
		matrix.preRotate(self.rotation)
		rotatedBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, True)
		buf = ByteBuffer.allocate(rotatedBitmap.getByteCount())
		rotatedBitmap.copyPixelsToBuffer(buf)
		self.captured_data = buf.array().tostring()
		self.captured_size = (rotatedBitmap.getWidth(), rotatedBitmap.getHeight())
		Clock.schedule_once(self.load_texture, 0)

	def load_texture(self, *args):
		tex = None
		if self.captured_data and self.captured_size:
			tex = Texture.create(size=self.captured_size, colorfmt='rgba')
			tex.blit_buffer(self.captured_data, colorfmt='rgba', bufferfmt='ubyte')
			tex.flip_vertical()
			self.captured_texture = tex
		self.captured_data = self.captured_size = None
		self.captured_texture = tex
		self.dispatch('on_capture_completed', self.captured_texture, self.captured_size)

	def handle_shutter(self):
		def playsnd(dt):
			self.shutter_sound.play()
		if self.shutter_sound:
			Clock.schedule_once(playsnd, 0)

	def do_capture(self):
		''' Call this to capture an image '''

                Logger.info('CameraPreview: do_capture')

		if not self.camera:
			return

                Logger.info('CameraPreview: cameratakepicture')

		# Don't use raw or post-view calbacks since they are not always supported
		self.camera.takePicture(
			_CameraShutterCallback(self.handle_shutter),
			None,
			None,
			_CameraPictureCallback(self.save_jpeg))

	def do_focus(self, fpoint):
		''' Call this to set the camera focus. `fpoint` is the center of the focus rect requested '''
		if all([self.play, self.focus_supported]):
			size = self.size
			msize = self.focus_rect_size
			if all([self.camera, self.params, min(fpoint) > 0, min(size) > 0, max(msize) > 0]):
				def to_focus_space(v):
					# focus coordinate space is [-1000, 1000]
					return (boundary(v, 0., 1.) * 2000) - 1000
				def get_ArrayList(area):
					array = ArrayList()
					array.add(CameraArea(Rect(*area), self.default_focus_weight))
					return array
				# get translated rect and convert to focus range
				focus_area = map(to_focus_space, self._rect_to_relative(fpoint, msize, size))
				try:
					self.camera.cancelAutoFocus()
					self.params.setFocusMode(Parameters.FOCUS_MODE_AUTO)
					if self.metering_areas_supported:
						metering_area = map(to_focus_space, self._rect_to_relative(
							fpoint,
							map(operator.mul, msize, [self.metering_area_factor] * 2),
							size)
						)
						self.params.setMeteringAreas(get_ArrayList(metering_area))
					self.params.setFocusAreas(get_ArrayList(focus_area))
					self.camera.setParameters(self.params)
					self.camera.setAutoFocusMoveCallback(_CameraFocusMoveCallback(self.on_focus_moving))
					self.camera.autoFocus(_CameraFocusCallback(self.focus_completed))
					return
				except Exception as e:
					Logger.error('CameraPreview: on_focus() exception %s' % e)

		self.dispatch('on_focus_completed', False)
		return False

	def on_focus_moving(self, start):
		if not self.focus_move_sound:
			return
		def moving(dt):
			if start:
				self.focus_move_sound.play()
			else:
				self.focus_move_sound.stop()
		Clock.schedule_once(moving, 0)

	def focus_completed(self, success):
		if success:
			def playsnd(dt):
				self.focus_done_sound.play()
			if self.focus_done_sound:
				Clock.schedule_once(playsnd, 0)
		self.dispatch('on_focus_completed', success)

	def resolve_size(self, size, sizes):
		if sizes:
			if size in sizes:
				return size
			elif list(reversed(size)) in sizes:
				return list(reversed(size))
			elif self.prefer_max_size_if_not_specified:
				areas = dict(((s[0]*s[1]), s) for s in sizes)
				if areas:
					return areas[sorted(areas.keys())[-1]]
		return []

	def resolve_picture_size(self, *args):
		self.picture_size = self.resolve_size(
			self.preferred_picture_size,
			self.supported_picture_sizes)

	def resolve_preview_size(self, *args):
		self.preview_size = self.resolve_size(
			self.preferred_preview_size,
			self.supported_preview_sizes)

	def on_preview_size(self, instance, value):
		if self.params:
			self.params.setPreviewSize(*value)
			if self.camera:
				self.camera.setParameters(self.params)

	def on_picture_size(self, instance, value):
		if self.params:
			self.params.setPictureSize(*value)
			if self.camera:
				self.camera.setParameters(self.params)

	def release_camera(self):
		try:
			if self.camera:
				self.camera.release()
		except Exception as e:
			Logger.warning('CameraPreview: release() exception %s' % e)

	def on_camera_id(self, instance, value):
		self.release_camera()
		self.init_camera()

	def disable_shutter_sound(self):
		if api_version >= 17 and self.info.canDisableShutterSound:
			self.camera.enableShutterSound(False)
		else:
			# if that's not possible, don't handle it ourselves in order to prevent a double shutter sound
			self.shutter_sound = None

	def init_camera(self):
		try:
			self.camera = AndroidCamera.open(self.camera_id)
			if self.camera:
				self.info = CameraInfo()
				AndroidCamera.getCameraInfo(self.camera_id, self.info)
				# detect display rotation and camera placement and set orientation accordingly
				self.set_display_orientation()
				# load parameters and update as required
				self.update_parameters()
				# try to disable shutter sound in case we handle it ourselves
				self.disable_shutter_sound()
				return True
		except Exception as e:
			Logger.error('CameraPreview: camera initialization failed: %s' % e)

	def on_play(self, instance, value):
		if value:
			if self.init_camera():
				self.start_preview()
		else:
			self.stop_preview()
			self.release_camera()

	def on_flash_mode(self, instance, value):
		if self.camera and self.params:
			if value not in self._flash_mode_to_android:
				Logger.warning('CameraPreview: flash mode `%s` not supported' % value)
				return
			self.params.setFlashMode(self._flash_mode_to_android[value])
			self.camera.setParameters(self.params)

	def update_parameters(self):
		if not self.camera:
			return
		self.params = params = self.camera.getParameters()
		if params is None:
			Logger.error('CameraPreview: can''t read parameters')
			return
		#
		# Preview settings
		#
		params.setAutoExposureLock(False)
		params.setVideoStabilization(True)
		params.setPreviewFpsRange(30000, 30000)
		#
		# Capture settings
		#
		params.setJpegQuality(90)
		params.setRotation(self.rotation)
		self.camera.setParameters(params)
		# get preview and picture size support
		self.supported_picture_sizes = map_List(
			lambda x: [x.width, x.height],
			params.getSupportedPictureSizes())
		self.supported_preview_sizes = map_List(
			lambda x: [x.width, x.height],
			params.getSupportedPreviewSizes())
		# get focus support
		self.focus_areas_supported = params.getMaxNumFocusAreas() > 0
		self.metering_areas_supported = params.getMaxNumMeteringAreas() > 0
		self.supported_focus_modes = map_List(
			lambda x: x,
			params.getSupportedFocusModes())
		self.autofocus_supported = Parameters.FOCUS_MODE_AUTO in self.supported_focus_modes
		# get flash modes supported
		#self.supported_flash_modes = map_List(
		#	lambda x: x,
		#	params.getSupportedFlashModes())
		#self.flash_modes = set(self.supported_flash_modes).union(set(self.flash_modes))
		#if self.flash_mode in self.flash_modes:
		#	self.on_flash_mode(None, self.flash_mode)

	def set_display_orientation(self):
		rotation = theActivity.getWindowManager().getDefaultDisplay().getRotation()
		degrees = self._rotation_to_degrees[rotation]
		if self.info.facing == CameraInfo.CAMERA_FACING_FRONT:
			result = (self.info.orientation + degrees) % 360
			result = (360 - result) % 360;  # compensate the mirror
		else: # back-facing
			result = (self.info.orientation - degrees + 360) % 360
		self.rotation = result
		self.camera.setDisplayOrientation(result)

	def start_preview(self):
		''' Starts camera preview. Requires some way of getting a surface holder from the Activity '''
		if self.camera:
			try:
				holder = theActivity.getCameraSurfaceHolder()
				if not holder:
					Logger.warning('Camera: no holder, can''t start preview')
				self.camera.setPreviewDisplay(holder)
				self.camera.startPreview()
				pass
			except Exception as e:
				Logger.error('CameraPreview: startPreview() exception %s' % e)

	def stop_preview(self):
		if self.camera:
			try:
				self.camera.stopPreview()
				pass
			except Exception as e:
				Logger.error('CameraPreview: stopPreview() exception %s' % e)


	def on_capture_completed(self):
                pass

        def on_focus_completed(self):
                pass


