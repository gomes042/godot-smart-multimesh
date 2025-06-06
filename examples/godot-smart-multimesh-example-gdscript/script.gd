extends SmartMultiMeshInstance3D

func init() -> void:
	print("init() GDSCRIPT")
	
	var container_spacing_x: float = 10.0  # Distance between containers (X)
	var wrap_height: int = 50  # Max number of instances before wrapping to next column (Y limit)
	
	# Spacing between instances in X and Y
	var gap_x: float = 2.0
	var gap_y: float = 2.0

	var container_offset_x: float = 0.0  # Tracks where to start each container on the X-axis

	for container in containers:
		print(" ")

		var container_index := get_container_index(container)
		print("Container Index (get_container_index): ", container_index)
		print("Container Index (container.get_index): ", container.get_index())
		print("Instances in Container: ", container.instance_count)
		
		for s in range(container.instance_count):
			var column: int = s / wrap_height  # Which column inside this container
			var row: int = s % wrap_height     # Which row (Y)

			var x: float = container_offset_x + float(column) * gap_x
			var y: float = float(row) * gap_y

			var pos: Vector3 = Vector3(x, y, 1.0)
			container.set_instance_transform(s, Transform3D(Basis.IDENTITY, pos))
		
		# Move container offset X to the right for the next container,
		# considering how many columns were used
		var num_columns: int = ceil(float(container.instance_count) / float(wrap_height))
		container_offset_x += float(num_columns) * gap_x + container_spacing_x
		#print("instance: ", s, " position: ", pos)
		
		#container.for_every_instance(func(instance_index):
		#	print("Processing instance ", instance_index)
		#)


func _on_button_cube_add_pressed(q=1) -> void:
	containers[0].instance_count += q;
	init() #TODO: replace this line

func _on_button_prism_add_pressed(q=1) -> void:
	containers[1].instance_count += q;
	init() #TODO: replace this line

func _on_button_capsule_add_pressed(q=1) -> void:
	containers[2].instance_count += q;
	init() #TODO: replace this line

func _on_button_cube_remove_pressed() -> void:
	if containers[0].instance_count == 0: return
	containers[0].instance_count -= 10;
	init() #TODO: replace this line

func _on_button_prism_remove_pressed() -> void:
	if containers[1].instance_count == 0: return
	containers[1].instance_count -= 10;
	init() #TODO: replace this line

func _on_button_capsule_remove_pressed() -> void:
	if containers[2].instance_count == 0: return
	containers[2].instance_count -= 10;
	init() #TODO: replace this line
