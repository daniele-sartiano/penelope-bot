//
// Created by lele on 22/05/20.
//

#include <model.h>
#include "gtest/gtest.h"

class ModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::set<std::string> s;
        s.insert("www.other.it");
        s.insert("www.another.it");
        m = new Model(1590162776, "www.test.it", "the text of the page", "page.www.test.it.out", s);
    }

    Model* m{};
};

TEST_F(ModelTest, CheckFields) {
    EXPECT_EQ(m->getLink(), "www.test.it");
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
return RUN_ALL_TESTS();
}