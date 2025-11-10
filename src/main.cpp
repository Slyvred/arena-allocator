#include "arena_list.hpp"
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "logger.hpp"

/* This file is contains a few examples on how to use the Arena */

struct Point2D {
    int x;
    int y;

    Point2D(int x, int y): x(x), y(y) {}
};

std::ostream& operator<<(std::ostream& os, const Point2D& pt) {
    os << "Point2D{\n x: " << pt.x << ",\n y: " << pt.y << "\n}";
    return os;
}

struct Point3D {
    double x;
    double y;
    double z;

    Point3D(double x, double y, double z): x(x), y(y), z(z) {}
};

Logger logger = Logger(Logger::LogLevel::DEBUG, "%m/%d/%y %H:%M:%S");

int main(int argc, char** argv) {
    Arena arena = Arena(8);

    Point2D* points[3];
    for (int i = 0; i < 3; i++) {
        // auto* ptr = arena.allocate(sizeof(Point2D), alignof(Point2D));
        // points[i] = new (ptr) Point2D({2*i, 3*i});
        points[i] = arena.make<Point2D>(2*i, 3*i);
        std::cout << *points[i] << "\n";
    }

    arena.reset();

    // auto* ptr = arena.allocate(64 * sizeof(char), alignof(char));


    for (int i = 0; i < 3; i++) {
        // auto* ptr = arena.allocate(sizeof(Point2D), alignof(Point2D));
        // points[i] = new (ptr) Point2D({20*i+1, 30*i+1});
        points[i] = arena.make<Point2D>(20*i+1, 30*i+1);
        std::cout << *points[i] << "\n";
    }

    ArenaAllocator<int> alloc(&arena);
    std::vector<int, ArenaAllocator<int>> v(alloc);

    for (int i = 0; i < 64; i++)
        v.push_back(i);

    for (int x : v)
        printf("%d ", x);
    printf("\n");

    ArenaAllocator<std::pair<const int,int>> my_alloc(&arena);

    // std::unordered_map using our ArenaAllocator
    std::unordered_map<int,int,
        std::hash<int>,
        std::equal_to<int>,
        ArenaAllocator<std::pair<const int,int>>
    > umap(0, std::hash<int>(), std::equal_to<int>(), my_alloc);

    for (int i = 0; i < 12; i++) {
        umap[i] = i;
    }

    return 0;
}
