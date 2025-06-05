@tool
extends SmartMultiMeshInstance3D

func init() -> void:
	print("init() GDSCRIPT")
	
	for container in containers:
		print(" ")

		var container_index := get_container_index(container)
		print("Container Index (get_container_index): ", container_index)
		print("Container Index (container.get_index): ", container.get_index())
		print("Instances in Container: ", container.instance_count)
		
		var space_x := float((container_index) * 3)
		
		for s in range(container.instance_count):
			var x := space_x + float(container_index * 10 + s)  # Spread containers along X, instances add spacing
			var y := float(container_index * 2 + s % 2)  # Some variation in Y using container and instance
			var pos := Vector3(x, y, 1.0)
			
			container.set_instance_transform(s, Transform3D(Basis.IDENTITY, pos))
			#print("instance: ", s, " position: ", pos)
		
		#container.for_every_instance(func(instance_index):
		#	print("Processing instance ", instance_index)
		#)
