for i=1:8
    ptcloud = pcread(['point_cloud' num2str(i) '.pcd']);
    figure()
    pcshow(ptcloud);
end