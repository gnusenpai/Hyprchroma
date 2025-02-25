#pragma once

#include <string>
#include <format>

#include <hyprland/src/render/shaders/Textures.hpp>


inline static const std::string DARK_MODE_FUNC = R"glsl(
uniform bool doInvert;
uniform vec3 bkg;

void invert(inout vec4 color) {
    if (doInvert) {
        // true chroma key shader
        // from https://www.shadertoy.com/view/XsjyDy

        vec3 chroma = vec3(0,1,0);

        float level = 0.5;
        float threshold = 0.35;

        // normalize colors
        // normLength is used to filter false color matches
        float normLength = clamp(length(color.rgb) / length(chroma.rgb), 0.0, 1.0);
        vec3 normColor = normalize(color.rgb) * normLength;
        vec3 normKey = normalize(chroma.rgb);

        // Calculate difference from KEY_COLOR
        float colorDiff = length(normColor - normKey);

        if (color.rgb == vec3(0,0,0)) {
            // fix pure black hack
            colorDiff = 1.;
        } else {
            // remove green edge glow
            colorDiff = smoothstep(level - threshold, level + threshold, colorDiff);
            color.g = color.g - (chroma.g * (1.0 - colorDiff));
        }

        // NOTE!
        // In reality, filtered image should not be mixed, but rendered in a seperate pass
        // This will also enable removal of dark alpha edges (maybe that is the buffer functionality)

        color.a = colorDiff;
    }
}
)glsl";


inline const std::string TEXFRAGSRCRGBA_DARK = R"glsl(
precision mediump float;
varying vec2 v_texcoord; // is in 0-1
uniform sampler2D tex;
uniform float alpha;

uniform vec2 topLeft;
uniform vec2 fullSize;
uniform float radius;

uniform int discardOpaque;
uniform int discardAlpha;
uniform float discardAlphaValue;

uniform int applyTint;
uniform vec3 tint;

)glsl" + DARK_MODE_FUNC + R"glsl(

void main() {

    vec4 pixColor = texture2D(tex, v_texcoord);

    if (discardOpaque == 1 && pixColor[3] * alpha == 1.0)
	    discard;

    if (discardAlpha == 1 && pixColor[3] <= discardAlphaValue)
        discard;

    if (applyTint == 2) {
	    pixColor[0] = pixColor[0] * tint[0];
	    pixColor[1] = pixColor[1] * tint[1];
	    pixColor[2] = pixColor[2] * tint[2];
    }

    invert(pixColor);

    if (radius > 0.0) {
    )glsl" +
    ROUNDED_SHADER_FUNC("pixColor") + R"glsl(
    }

    gl_FragColor = pixColor * alpha;
})glsl";

inline const std::string TEXFRAGSRCRGBX_DARK = R"glsl(
precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex;
uniform float alpha;

uniform vec2 topLeft;
uniform vec2 fullSize;
uniform float radius;

uniform int discardOpaque;
uniform int discardAlpha;
uniform int discardAlphaValue;

uniform int applyTint;
uniform vec3 tint;

)glsl" + DARK_MODE_FUNC + R"glsl(

void main() {

    if (discardOpaque == 1 && alpha == 1.0)
	discard;

    vec4 pixColor = vec4(texture2D(tex, v_texcoord).rgb, 1.0);

    if (applyTint == 2) {
        pixColor[0] = pixColor[0] * tint[0];
        pixColor[1] = pixColor[1] * tint[1];
        pixColor[2] = pixColor[2] * tint[2];
    }

    invert(pixColor);

    if (radius > 0.0) {
    )glsl" +
    ROUNDED_SHADER_FUNC("pixColor") + R"glsl(
    }

    gl_FragColor = pixColor * alpha;
})glsl";

inline const std::string TEXFRAGSRCEXT_DARK = R"glsl(
#extension GL_OES_EGL_image_external : require

precision mediump float;
varying vec2 v_texcoord;
uniform samplerExternalOES texture0;
uniform float alpha;

uniform vec2 topLeft;
uniform vec2 fullSize;
uniform float radius;

uniform int discardOpaque;
uniform int discardAlpha;
uniform int discardAlphaValue;

uniform int applyTint;
uniform vec3 tint;

)glsl" + DARK_MODE_FUNC + R"glsl(

void main() {

    vec4 pixColor = texture2D(texture0, v_texcoord);

    if (discardOpaque == 1 && pixColor[3] * alpha == 1.0)
        discard;

    if (applyTint == 2) {
        pixColor[0] = pixColor[0] * tint[0];
        pixColor[1] = pixColor[1] * tint[1];
        pixColor[2] = pixColor[2] * tint[2];
    }

    invert(pixColor);

    if (radius > 0.0) {
    )glsl" +
    ROUNDED_SHADER_FUNC("pixColor") + R"glsl(
    }

    gl_FragColor = pixColor * alpha;
}
)glsl";
