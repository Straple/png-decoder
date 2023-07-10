#pragma once

#include <iostream>
#include <vector>

struct RGB {
    int r = 0, g = 0, b = 0, a = 0;
    bool operator==(const RGB &rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
    }
};

inline std::ostream &operator<<(std::ostream &out, const RGB &x) {
    out << x.r << " " << x.g << " " << x.b << " " << x.a;
    return out;
}

class Image {
public:
    Image() {}
    Image(int height, int width) {
        SetSize(height, width);
    }

    void SetSize(int height, int width) {
        height_ = height;
        width_ = width;
        data_.resize(height_ * width_);
    }

    const RGB &operator()(int row, int col) const {
        return data_[width_ * row + col];
    }

    RGB &operator()(int row, int col) {
        return data_[width_ * row + col];
    }

    int Height() const {
        return height_;
    }

    int Width() const {
        return width_;
    }

private:
    std::vector<RGB> data_;
    int height_ = 0;
    int width_ = 0;
};
