package com.samsung.wearable_rotary

import android.view.MotionEvent
import androidx.core.view.InputDeviceCompat
import androidx.core.view.MotionEventCompat
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.EventChannel

/** WearableRotaryPlugin */
class WearableRotaryPlugin : FlutterPlugin, EventChannel.StreamHandler {
    companion object {
        private var events: EventChannel.EventSink? = null

        fun onGenericMotionEvent(event: MotionEvent?): Boolean {
            return if (event != null && event.action == MotionEvent.ACTION_SCROLL && event.isFromSource(
                    InputDeviceCompat.SOURCE_ROTARY_ENCODER
                )
            ) {
                events?.success(event.getAxisValue(MotionEventCompat.AXIS_SCROLL))
                true
            } else {
                false
            }
        }
    }

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        val channel = EventChannel(flutterPluginBinding.binaryMessenger, "flutter.wearable_rotary.channel")
        channel.setStreamHandler(this)
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {}

    override fun onListen(arguments: Any?, events: EventChannel.EventSink?) {
        Companion.events = events
    }

    override fun onCancel(arguments: Any?) {
        events = null
    }
}
