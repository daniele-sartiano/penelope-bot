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
        m = new Model(1590162776, "www.test.it", "the text of the page", "page.www.test.it.out", "191.162.1.1", s);
    }

    Model* m{};
};

TEST_F(ModelTest, CheckFields) {
    EXPECT_EQ(m->get_link(), "www.test.it");
    EXPECT_EQ(m->get_text(), "the text of the page");
}


TEST_F(ModelTest, CheckMultipleSerDes) {
    std::string s1 = m->serialize();
    std::string s2 = m->serialize();
    std::string s3 = "{\"models\":[";
    s3.append(s1);
    s3.append(",");
    s3.append(s2);
    s3.append("]}");
    std::vector<Model> models = Model::deserialize_models(s3);
    EXPECT_EQ(models.size(), 2);
    Model m1 = models.front();
    EXPECT_EQ(m1.get_link(), m->get_link());
    std::string s4 = Model::serialize_models(models);
    EXPECT_EQ(s4, s3);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
return RUN_ALL_TESTS();
}