package com.samsung.wearable_rotary_example

import android.view.MotionEvent
import com.samsung.wearable_rotary.WearableRotaryPlugin
import io.flutter.embedding.android.FlutterActivity

class MainActivity: FlutterActivity() {
    override fun onGenericMotionEvent(event: MotionEvent?): Boolean {
        val handled = WearableRotaryPlugin.onGenericMotionEvent(event)
        return if (handled) {
            true
        } else {
            super.onGenericMotionEvent(event)
        }
    }
}
