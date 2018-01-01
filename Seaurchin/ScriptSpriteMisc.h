#pragma once

struct Transform2D {
    double X = 0.0;
    double Y = 0.0;
    double Angle = 0.0;
    double OriginX = 0.0;
    double OriginY = 0.0;
    double ScaleX = 1.0;
    double ScaleY = 1.0;
    Transform2D ApplyFrom(const Transform2D &parent)
    {
        Transform2D result;
        // Origin�͉摜��̈ʒu�Ȃ̂ŕύX�Ȃ�
        result.OriginX = OriginX;
        result.OriginY = OriginY;
        // �������Ȃ��͕̂S�����m���������Ă���
        result.ScaleX = parent.ScaleX * ScaleX;
        result.ScaleY = parent.ScaleY * ScaleY;
        // ��������͐������Ǝv��
        result.Angle = parent.Angle + Angle;
        double rx = X * parent.ScaleX, ry = Y * parent.ScaleY;
        result.X = parent.X + (rx * cos(parent.Angle) - ry * sin(parent.Angle));
        result.Y = parent.Y + (rx * sin(parent.Angle) + ry * cos(parent.Angle));
        return result;
    }
};

struct Transform3D {
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;
    double AngleX = 0.0;
    double AngleY = 0.0;
    double AngleZ = 0.0;
};

struct ColorTint {
    unsigned char A;
    unsigned char R;
    unsigned char G;
    unsigned char B;
    ColorTint ApplyFrom(const ColorTint &parent)
    {
        double na = ((int)A * (int)parent.A) / 255.0;
        double nr = ((int)R * (int)parent.R) / 255.0;
        double ng = ((int)G * (int)parent.G) / 255.0;
        double nb = ((int)B * (int)parent.B) / 255.0;
        ColorTint result = {
            (unsigned char)na,
            (unsigned char)nr,
            (unsigned char)ng,
            (unsigned char)nb,
        };
        return result;
    }
} ;

class Colors final {
public:
    static constexpr ColorTint AliceBlue = { 0xFF, 0xF0, 0xF8, 0xFF };
    static constexpr ColorTint AntiqueWhite = { 0xFF, 0xFA, 0xEB, 0xD7 };
    static constexpr ColorTint Aqua = { 0xFF, 0x00, 0xFF, 0xFF };
    static constexpr ColorTint Aquamarine = { 0xFF, 0x7F, 0xFF, 0xD4 };
    static constexpr ColorTint Azure = { 0xFF, 0xF0, 0xFF, 0xFF };
    static constexpr ColorTint Beige = { 0xFF, 0xF5, 0xF5, 0xDC };
    static constexpr ColorTint Bisque = { 0xFF, 0xFF, 0xE4, 0xC4 };
    static constexpr ColorTint Black = { 0xFF, 0x00, 0x00, 0x00 };
    static constexpr ColorTint BlanchedAlmond = { 0xFF, 0xFF, 0xEB, 0xCD };
    static constexpr ColorTint Blue = { 0xFF, 0x00, 0x00, 0xFF };
    static constexpr ColorTint BlueViolet = { 0xFF, 0x8A, 0x2B, 0xE2 };
    static constexpr ColorTint Brown = { 0xFF, 0xA5, 0x2A, 0x2A };
    static constexpr ColorTint BurlyWood = { 0xFF, 0xDE, 0xB8, 0x87 };
    static constexpr ColorTint CadetBlue = { 0xFF, 0x5F, 0x9E, 0xA0 };
    static constexpr ColorTint Chartreuse = { 0xFF, 0x7F, 0xFF, 0x00 };
    static constexpr ColorTint Chocolate = { 0xFF, 0xD2, 0x69, 0x1E };
    static constexpr ColorTint Coral = { 0xFF, 0xFF, 0x7F, 0x50 };
    static constexpr ColorTint CornflowerBlue = { 0xFF, 0x64, 0x95, 0xED };
    static constexpr ColorTint Cornsilk = { 0xFF, 0xFF, 0xF8, 0xDC };
    static constexpr ColorTint Crimson = { 0xFF, 0xDC, 0x14, 0x3C };
    static constexpr ColorTint Cyan = { 0xFF, 0x00, 0xFF, 0xFF };
    static constexpr ColorTint DarkBlue = { 0xFF, 0x00, 0x00, 0x8B };
    static constexpr ColorTint DarkCyan = { 0xFF, 0x00, 0x8B, 0x8B };
    static constexpr ColorTint DarkGoldenrod = { 0xFF, 0xB8, 0x86, 0x0B };
    static constexpr ColorTint DarkGray = { 0xFF, 0xA9, 0xA9, 0xA9 };
    static constexpr ColorTint DarkGreen = { 0xFF, 0x00, 0x64, 0x00 };
    static constexpr ColorTint DarkKhaki = { 0xFF, 0xBD, 0xB7, 0x6B };
    static constexpr ColorTint DarkMagenta = { 0xFF, 0x8B, 0x00, 0x8B };
    static constexpr ColorTint DarkOliveGreen = { 0xFF, 0x55, 0x6B, 0x2F };
    static constexpr ColorTint DarkOrange = { 0xFF, 0xFF, 0x8C, 0x00 };
    static constexpr ColorTint DarkOrchid = { 0xFF, 0x99, 0x32, 0xCC };
    static constexpr ColorTint DarkRed = { 0xFF, 0x8B, 0x00, 0x00 };
    static constexpr ColorTint DarkSalmon = { 0xFF, 0xE9, 0x96, 0x7A };
    static constexpr ColorTint DarkSeaGreen = { 0xFF, 0x8F, 0xBC, 0x8F };
    static constexpr ColorTint DarkSlateBlue = { 0xFF, 0x48, 0x3D, 0x8B };
    static constexpr ColorTint DarkSlateGray = { 0xFF, 0x2F, 0x4F, 0x4F };
    static constexpr ColorTint DarkTurquoise = { 0xFF, 0x00, 0xCE, 0xD1 };
    static constexpr ColorTint DarkViolet = { 0xFF, 0x94, 0x00, 0xD3 };
    static constexpr ColorTint DeepPink = { 0xFF, 0xFF, 0x14, 0x93 };
    static constexpr ColorTint DeepSkyBlue = { 0xFF, 0x00, 0xBF, 0xFF };
    static constexpr ColorTint DimGray = { 0xFF, 0x69, 0x69, 0x69 };
    static constexpr ColorTint DodgerBlue = { 0xFF, 0x1E, 0x90, 0xFF };
    static constexpr ColorTint Firebrick = { 0xFF, 0xB2, 0x22, 0x22 };
    static constexpr ColorTint FloralWhite = { 0xFF, 0xFF, 0xFA, 0xF0 };
    static constexpr ColorTint ForestGreen = { 0xFF, 0x22, 0x8B, 0x22 };
    static constexpr ColorTint Fuchsia = { 0xFF, 0xFF, 0x00, 0xFF };
    static constexpr ColorTint Gainsboro = { 0xFF, 0xDC, 0xDC, 0xDC };
    static constexpr ColorTint GhostWhite = { 0xFF, 0xF8, 0xF8, 0xFF };
    static constexpr ColorTint Gold = { 0xFF, 0xFF, 0xD7, 0x00 };
    static constexpr ColorTint Goldenrod = { 0xFF, 0xDA, 0xA5, 0x20 };
    static constexpr ColorTint Gray = { 0xFF, 0x80, 0x80, 0x80 };
    static constexpr ColorTint Green = { 0xFF, 0x00, 0x80, 0x00 };
    static constexpr ColorTint GreenYellow = { 0xFF, 0xAD, 0xFF, 0x2F };
    static constexpr ColorTint Honeydew = { 0xFF, 0xF0, 0xFF, 0xF0 };
    static constexpr ColorTint HotPink = { 0xFF, 0xFF, 0x69, 0xB4 };
    static constexpr ColorTint IndianRed = { 0xFF, 0xCD, 0x5C, 0x5C };
    static constexpr ColorTint Indigo = { 0xFF, 0x4B, 0x00, 0x82 };
    static constexpr ColorTint Ivory = { 0xFF, 0xFF, 0xFF, 0xF0 };
    static constexpr ColorTint Khaki = { 0xFF, 0xF0, 0xE6, 0x8C };
    static constexpr ColorTint Lavender = { 0xFF, 0xE6, 0xE6, 0xFA };
    static constexpr ColorTint LavenderBlush = { 0xFF, 0xFF, 0xF0, 0xF5 };
    static constexpr ColorTint LawnGreen = { 0xFF, 0x7C, 0xFC, 0x00 };
    static constexpr ColorTint LemonChiffon = { 0xFF, 0xFF, 0xFA, 0xCD };
    static constexpr ColorTint LightBlue = { 0xFF, 0xAD, 0xD8, 0xE6 };
    static constexpr ColorTint LightCoral = { 0xFF, 0xF0, 0x80, 0x80 };
    static constexpr ColorTint LightCyan = { 0xFF, 0xE0, 0xFF, 0xFF };
    static constexpr ColorTint LightGoldenrodYellow = { 0xFF, 0xFA, 0xFA, 0xD2 };
    static constexpr ColorTint LightGray = { 0xFF, 0xD3, 0xD3, 0xD3 };
    static constexpr ColorTint LightGreen = { 0xFF, 0x90, 0xEE, 0x90 };
    static constexpr ColorTint LightPink = { 0xFF, 0xFF, 0xB6, 0xC1 };
    static constexpr ColorTint LightSalmon = { 0xFF, 0xFF, 0xA0, 0x7A };
    static constexpr ColorTint LightSeaGreen = { 0xFF, 0x20, 0xB2, 0xAA };
    static constexpr ColorTint LightSkyBlue = { 0xFF, 0x87, 0xCE, 0xFA };
    static constexpr ColorTint LightSlateGray = { 0xFF, 0x77, 0x88, 0x99 };
    static constexpr ColorTint LightSteelBlue = { 0xFF, 0xB0, 0xC4, 0xDE };
    static constexpr ColorTint LightYellow = { 0xFF, 0xFF, 0xFF, 0xE0 };
    static constexpr ColorTint Lime = { 0xFF, 0x00, 0xFF, 0x00 };
    static constexpr ColorTint LimeGreen = { 0xFF, 0x32, 0xCD, 0x32 };
    static constexpr ColorTint Linen = { 0xFF, 0xFA, 0xF0, 0xE6 };
    static constexpr ColorTint Magenta = { 0xFF, 0xFF, 0x00, 0xFF };
    static constexpr ColorTint Maroon = { 0xFF, 0x80, 0x00, 0x00 };
    static constexpr ColorTint MediumAquamarine = { 0xFF, 0x66, 0xCD, 0xAA };
    static constexpr ColorTint MediumBlue = { 0xFF, 0x00, 0x00, 0xCD };
    static constexpr ColorTint MediumOrchid = { 0xFF, 0xBA, 0x55, 0xD3 };
    static constexpr ColorTint MediumPurple = { 0xFF, 0x93, 0x70, 0xDB };
    static constexpr ColorTint MediumSeaGreen = { 0xFF, 0x3C, 0xB3, 0x71 };
    static constexpr ColorTint MediumSlateBlue = { 0xFF, 0x7B, 0x68, 0xEE };
    static constexpr ColorTint MediumSpringGreen = { 0xFF, 0x00, 0xFA, 0x9A };
    static constexpr ColorTint MediumTurquoise = { 0xFF, 0x48, 0xD1, 0xCC };
    static constexpr ColorTint MediumVioletRed = { 0xFF, 0xC7, 0x15, 0x85 };
    static constexpr ColorTint MidnightBlue = { 0xFF, 0x19, 0x19, 0x70 };
    static constexpr ColorTint MintCream = { 0xFF, 0xF5, 0xFF, 0xFA };
    static constexpr ColorTint MistyRose = { 0xFF, 0xFF, 0xE4, 0xE1 };
    static constexpr ColorTint Moccasin = { 0xFF, 0xFF, 0xE4, 0xB5 };
    static constexpr ColorTint NavajoWhite = { 0xFF, 0xFF, 0xDE, 0xAD };
    static constexpr ColorTint Navy = { 0xFF, 0x00, 0x00, 0x80 };
    static constexpr ColorTint OldLace = { 0xFF, 0xFD, 0xF5, 0xE6 };
    static constexpr ColorTint Olive = { 0xFF, 0x80, 0x80, 0x00 };
    static constexpr ColorTint OliveDrab = { 0xFF, 0x6B, 0x8E, 0x23 };
    static constexpr ColorTint Orange = { 0xFF, 0xFF, 0xA5, 0x00 };
    static constexpr ColorTint OrangeRed = { 0xFF, 0xFF, 0x45, 0x00 };
    static constexpr ColorTint Orchid = { 0xFF, 0xDA, 0x70, 0xD6 };
    static constexpr ColorTint PaleGoldenrod = { 0xFF, 0xEE, 0xE8, 0xAA };
    static constexpr ColorTint PaleGreen = { 0xFF, 0x98, 0xFB, 0x98 };
    static constexpr ColorTint PaleTurquoise = { 0xFF, 0xAF, 0xEE, 0xEE };
    static constexpr ColorTint PaleVioletRed = { 0xFF, 0xDB, 0x70, 0x93 };
    static constexpr ColorTint PapayaWhip = { 0xFF, 0xFF, 0xEF, 0xD5 };
    static constexpr ColorTint PeachPuff = { 0xFF, 0xFF, 0xDA, 0xB9 };
    static constexpr ColorTint Peru = { 0xFF, 0xCD, 0x85, 0x3F };
    static constexpr ColorTint Pink = { 0xFF, 0xFF, 0xC0, 0xCB };
    static constexpr ColorTint Plum = { 0xFF, 0xDD, 0xA0, 0xDD };
    static constexpr ColorTint PowderBlue = { 0xFF, 0xB0, 0xE0, 0xE6 };
    static constexpr ColorTint Purple = { 0xFF, 0x80, 0x00, 0x80 };
    static constexpr ColorTint Red = { 0xFF, 0xFF, 0x00, 0x00 };
    static constexpr ColorTint RosyBrown = { 0xFF, 0xBC, 0x8F, 0x8F };
    static constexpr ColorTint RoyalBlue = { 0xFF, 0x41, 0x69, 0xE1 };
    static constexpr ColorTint SaddleBrown = { 0xFF, 0x8B, 0x45, 0x13 };
    static constexpr ColorTint Salmon = { 0xFF, 0xFA, 0x80, 0x72 };
    static constexpr ColorTint SandyBrown = { 0xFF, 0xF4, 0xA4, 0x60 };
    static constexpr ColorTint SeaGreen = { 0xFF, 0x2E, 0x8B, 0x57 };
    static constexpr ColorTint SeaShell = { 0xFF, 0xFF, 0xF5, 0xEE };
    static constexpr ColorTint Sienna = { 0xFF, 0xA0, 0x52, 0x2D };
    static constexpr ColorTint Silver = { 0xFF, 0xC0, 0xC0, 0xC0 };
    static constexpr ColorTint SkyBlue = { 0xFF, 0x87, 0xCE, 0xEB };
    static constexpr ColorTint SlateBlue = { 0xFF, 0x6A, 0x5A, 0xCD };
    static constexpr ColorTint SlateGray = { 0xFF, 0x70, 0x80, 0x90 };
    static constexpr ColorTint Snow = { 0xFF, 0xFF, 0xFA, 0xFA };
    static constexpr ColorTint SpringGreen = { 0xFF, 0x00, 0xFF, 0x7F };
    static constexpr ColorTint SteelBlue = { 0xFF, 0x46, 0x82, 0xB4 };
    static constexpr ColorTint Tan = { 0xFF, 0xD2, 0xB4, 0x8C };
    static constexpr ColorTint Teal = { 0xFF, 0x00, 0x80, 0x80 };
    static constexpr ColorTint Thistle = { 0xFF, 0xD8, 0xBF, 0xD8 };
    static constexpr ColorTint Tomato = { 0xFF, 0xFF, 0x63, 0x47 };
    static constexpr ColorTint Turquoise = { 0xFF, 0x40, 0xE0, 0xD0 };
    static constexpr ColorTint Violet = { 0xFF, 0xEE, 0x82, 0xEE };
    static constexpr ColorTint Wheat = { 0xFF, 0xF5, 0xDE, 0xB3 };
    static constexpr ColorTint White = { 0xFF, 0xFF, 0xFF, 0xFF };
    static constexpr ColorTint WhiteSmoke = { 0xFF, 0xF5, 0xF5, 0xF5 };
    static constexpr ColorTint Yellow = { 0xFF, 0xFF, 0xFF, 0x00 };
    static constexpr ColorTint YellowGreen = { 0xFF, 0x9A, 0xCD, 0x32 };
};
