import wips

test_pose3_0 = wips.pose3()

test_pose3_0.x = 10.0
test_pose3_0.y = 5.0
test_pose3_0.z = 1.0
test_pose3_0.wq = 5.0
test_pose3_0.xq = 10.0
test_pose3_0.yq = 10.0
test_pose3_0.zq = 10.0

blob = wips.pose3.encode(test_pose3_0)
blob.offset = 0
test_pose3_1 = wips.pose3.decode(blob)

print(test_pose3_1.x)
print(test_pose3_1.y)
print(test_pose3_1.z)
print(test_pose3_1.wq)
print(test_pose3_1.xq)
print(test_pose3_1.yq)
print(test_pose3_1.zq)

print(test_pose3_1.extract())

#TODO: Serde causes segfaults, figure out why
