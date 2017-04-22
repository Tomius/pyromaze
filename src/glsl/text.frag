// Copyright (c) 2014, Tamas Csala

varying vec2 vTexCoord;
uniform sampler2D uTex;
uniform vec4 uColor;

void main() {
  gl_FragColor = vec4(uColor.rgb, uColor.a * texture2D(uTex, vTexCoord).r)*0.01 + vec4(vTexCoord.rg, 0, 1);
}
