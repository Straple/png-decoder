#include "test_commons.hpp"
#include <catch.hpp>

TEST_CASE("logo") {
    CheckImage("logo.png", "out.png");
}

TEST_CASE("grayscale") {
    CheckImage("lenna_grayscale.png");
}

TEST_CASE("index") {
    CheckImage("lenna_index.png");
}

TEST_CASE("alpha") {
    CheckImage("logo_alpha.png");
}

TEST_CASE("bit") {
    CheckImage("1.png", "out.png");
}

TEST_CASE("interlace") {
    CheckImage("inter.png");
}

TEST_CASE("grayscale_alpha_interlace") {
    CheckImage("alpha_grayscale.png");
}

TEST_CASE("bad_crc") {
    CHECK_THROWS(CheckImage("crc.png"));
}

TEST_CASE("smile_plte") {
    CheckImage("smile_plte.png", "out.png");
}

TEST_CASE("bulletproof") {
    CheckImage("bulletproof.png", "out.png");
}

TEST_CASE("bulletproof_64") {
    CheckImage("bulletproof_64.png", "out.png");
}

TEST_CASE("bulletproof_mono") {
    CheckImage("bulletproof_mono.png", "out.png");
}

TEST_CASE("white1") {
    CheckImage("white1.png", "out.png");

    CheckImage("my_bw.png");
    CheckImage("my1.png");
    CheckImage("small1.png", "out.png");
}

TEST_CASE("unable_to_open") {
    CHECK_THROWS(CheckImage("not_found1273612536asduashydgwayd.png"));
}

#include <fstream>

TEST_CASE("slice") {
    /*std::ifstream in("logo.png", std::ios_base::in | std::ios_base::binary);
    if (!in.is_open()) {
        std::cout << "FATAL\n";
    }
    std::ofstream out("logo_bad.png", std::ios_base::out | std::ios_base::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(in), {});
    std::cout << buffer.size() << '\n';
    for (int i = 0; i < buffer.size(); i++) {
        char c = buffer[i];
        if (i > 5000) {
            c--;
        }
        out.write(&c, 1);
    }*/
    /*while(true){
        char c;
        if(!(in >> c)){
            break;
        }
        out.write(&c, 1);
    }*/
    //CheckImage("logo_slice.png");
    //CheckImage("empty.png");
    //CheckImage("logo_bad_crc.png");
}
