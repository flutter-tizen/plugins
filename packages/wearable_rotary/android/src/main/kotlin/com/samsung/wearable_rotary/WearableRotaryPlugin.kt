package com.samsung.wearable_rotary

import android.view.MotionEvent
import android.view.ViewConfiguration
import androidx.core.view.InputDeviceCompat
import androidx.core.view.MotionEventCompat
import androidx.core.view.ViewConfigurationCompat
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.EventChannel
import kotlin.math.abs
import kotlin.properties.Delegates

/** WearableRotaryPlugin */
class WearableRotaryPlugin : FlutterPlugin, EventChannel.StreamHandler {
    companion object {
        private var scaleFactor by Delegates.notNull<Float>()
        private var events: EventChannel.EventSink? = null

        fun onGenericMotionEvent(event: MotionEvent?): Boolean {
            if (event == null || event.action != MotionEvent.ACTION_SCROLL || !event.isFromSource(
                    InputDeviceCompat.SOURCE_ROTARY_ENCODER
                )
            ) {
                return false
            }

            val delta = event.getAxisValue(MotionEventCompat.AXIS_SCROLL) * scaleFactor
            events?.success(
                mapOf(
                    "direction" to if (delta < 0) "clockwise" else "counterClockwise",
                    "magnitude" to abs(delta),
                )
            )
            return true

        }
    }

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        val channel =
            EventChannel(flutterPluginBinding.binaryMessenger, "flutter.wearable_rotary.channel")
        channel.setStreamHandler(this)

        val context = flutterPluginBinding.applicationContext
        scaleFactor = ViewConfigurationCompat.getScaledVerticalScrollFactor(
            ViewConfiguration.get(context), context
        )
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {}

    override fun onListen(arguments: Any?, events: EventChannel.EventSink?) {
        Companion.events = events
    }

    override fun onCancel(arguments: Any?) {
        events = null
    }
}
