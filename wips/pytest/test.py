import wips

test_pose3_0 = wips.pose3()

test_pose3_0.x = 10.0
test_pose3_0.y = 5.0
test_pose3_0.z = 1.0
test_pose3_0.wq = 5.0
test_pose3_0.xq = 10.0
test_pose3_0.yq = 10.0
test_pose3_0.zq = 10.0

print(test_pose3_0.x)
print(test_pose3_0.y)
print(test_pose3_0.z)
print(test_pose3_0.wq)
print(test_pose3_0.xq)
print(test_pose3_0.yq)
print(test_pose3_0.zq)

#TODO: Serde causes segfaults, figure out why
