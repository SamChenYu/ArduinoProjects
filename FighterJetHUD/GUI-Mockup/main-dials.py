import tkinter as tk
import math
import time

# Setup window
root = tk.Tk()
canvas_width = 320
canvas_height = 240
window_width = canvas_width
window_height = canvas_height

screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()

x_pos = (screen_width // 2) - (window_width // 2)
y_pos = (screen_height // 2) - (window_height // 2)

root.geometry(f"{window_width}x{window_height}+{x_pos}+{y_pos}")
root.title("Bike HUD")

canvas = tk.Canvas(root, width=canvas_width, height=canvas_height, bg='black')
canvas.pack()

# Center coordinates
cx = canvas_width // 2
cy = canvas_height // 2


# UI Text Elements
heading_readout_text = canvas.create_text(cx+10, canvas_height - 20, text="", fill="lime", font=("Courier", 20, "bold"))
temp_text = canvas.create_text(40, canvas_height - 50, text="", fill="lime", font=("Courier", 10))
gforce_text = canvas.create_text(canvas_width - 40, canvas_height - 50, text="", fill="lime", font=("Courier", 10))
speed_text = canvas.create_text(75, canvas_height - 20, text="", fill="lime", font=("Courier", 10))

# HUD elements
crosshair_items = []
pitch_ladders = []
compass_labels = []
temp_bar_items = []
gforce_bar_items = []
speed_dial_items = []
altitude_dial_items = []



def rotate_point(x, y, angle_rad):
    cos_a = math.cos(angle_rad)
    sin_a = math.sin(angle_rad)
    return x * cos_a - y * sin_a, x * sin_a + y * cos_a


def draw_altitude_dial(altitude):
    for item in altitude_dial_items:
        canvas.delete(item)
    altitude_dial_items.clear()

    dial_radius = 15
    center_x = canvas_width - 30
    center_y = canvas_height - 23

    altitude_reading_x = canvas_width - 65
    altitude_reading_y = canvas_height - 20

    circle = canvas.create_oval(center_x - dial_radius, center_y - dial_radius,
                                center_x + dial_radius, center_y + dial_radius,
                                outline="lime", width=1)
    altitude_dial_items.append(circle)

    # Draw tick marks inside the dial
    # Draw tick marks inside the dial
    tick_count = 5
    for i in range(tick_count):
        tick_angle = -135 + i * (270 / (tick_count - 1))
        rad = math.radians(tick_angle)
        inner_r = dial_radius - 3
        outer_r = dial_radius
        x0 = center_x + inner_r * math.cos(rad)
        y0 = center_y + inner_r * math.sin(rad)
        x1 = center_x + outer_r * math.cos(rad)
        y1 = center_y + outer_r * math.sin(rad)
        tick = canvas.create_line(x0, y0, x1, y1, fill="lime", width=1)
        altitude_dial_items.append(tick)

    angle = (altitude % 200) * 2.7 - 135  # Map 0-200m to -135 to 135 degrees
    angle_rad = math.radians(angle)
    needle_length = dial_radius - 5
    x_end = center_x + needle_length * math.cos(angle_rad)
    y_end = center_y + needle_length * math.sin(angle_rad)

    needle = canvas.create_line(center_x, center_y, x_end, y_end, fill="lime", width=2)
    altitude_dial_items.append(needle)

    label = canvas.create_text(center_x, center_y + dial_radius + 10, text="m", fill="lime", font=("Courier", 7))
    altitude_dial_items.append(label)

    value_label = canvas.create_text(altitude_reading_x, altitude_reading_y, text=f"{int(altitude)}m", fill="lime", font=("Courier", 10))
    altitude_dial_items.append(value_label)





def draw_crosshair(roll_deg):
    for item in crosshair_items:
        canvas.delete(item)
    crosshair_items.clear()

    roll_rad = math.radians(roll_deg)

    # === Sizes ===
    r = 5             # central ring radius
    tail_len = 40     # horizontal tail length
    arc_radius = 40
    arc_offset = 0
    arc_width = 2
    chevron_size = 8

    # === Central Ring ===
    ring = canvas.create_oval(cx - r, cy - r, cx + r, cy + r, outline="lime", width=2)
    crosshair_items.append(ring)

    # === Horizontal Tails (rotated with roll) ===
    for dx in [-r - tail_len, r + tail_len]:
        x0, y0 = rotate_point(dx, 0, roll_rad)
        x1, y1 = rotate_point(dx + (-tail_len if dx < 0 else tail_len), 0, roll_rad)
        line = canvas.create_line(cx + x0, cy + y0, cx + x1, cy + y1, fill="lime", width=3)
        crosshair_items.append(line)

    # === Top and Bottom inward chevrons ===
    for angle_deg in [0, 180]:  # Left and right
        base_rad = math.radians(angle_deg + roll_deg)
        perp1_rad = base_rad + math.radians(20)
        perp2_rad = base_rad - math.radians(20)

        # Tip of chevron (outer point)
        tip_x = cx + (r + chevron_size) * math.cos(base_rad)
        tip_y = cy + (r + chevron_size) * math.sin(base_rad)

        # Wing points (closer to center)
        wing1_x = cx + r * math.cos(perp1_rad)
        wing1_y = cy + r * math.sin(perp1_rad)
        wing2_x = cx + r * math.cos(perp2_rad)
        wing2_y = cy + r * math.sin(perp2_rad)

        line1 = canvas.create_line(tip_x, tip_y, wing1_x, wing1_y, fill="lime", width=2)
        line2 = canvas.create_line(tip_x, tip_y, wing2_x, wing2_y, fill="lime", width=2)
        crosshair_items.extend([line1, line2])

    # === Optional: Thin inner crosshair (within the ring) ===
    inner_cross_len = 4
    for angle_deg in [0, 90]:
        rad = math.radians(angle_deg + roll_deg)
        dx = inner_cross_len * math.cos(rad)
        dy = inner_cross_len * math.sin(rad)
        line = canvas.create_line(cx - dx, cy - dy, cx + dx, cy + dy, fill="lime", width=1)
        crosshair_items.append(line)

    # === Static HUD Semi-Circles ===
    top_arc = canvas.create_arc(
        cx - arc_radius, cy - arc_radius - arc_offset,
        cx + arc_radius, cy + arc_radius - arc_offset,
        start=0, extent=180, style=tk.ARC,
        outline="lime", width=arc_width
    )
    bottom_arc = canvas.create_arc(
        cx - arc_radius, cy - arc_radius + arc_offset,
        cx + arc_radius, cy + arc_radius + arc_offset,
        start=180, extent=180, style=tk.ARC,
        outline="lime", width=arc_width
    )
    crosshair_items.extend([top_arc, bottom_arc])

    # === Arc Tick Marks ===
    tick_len = 5
    tick_spacing_deg = 15

    def draw_arc_ticks(cx, cy_arc, radius, start_angle, extent, spacing_deg):
        for angle in range(start_angle, start_angle + extent + 1, spacing_deg):
            rad = math.radians(angle)
            x_outer = cx + radius * math.cos(rad)
            y_outer = cy_arc + radius * math.sin(rad)

            dx = cx - x_outer
            dy = cy - y_outer
            dist = math.hypot(dx, dy)
            x_inner = x_outer + (dx / dist) * tick_len
            y_inner = y_outer + (dy / dist) * tick_len

            tick = canvas.create_line(x_outer, y_outer, x_inner, y_inner, fill="lime", width=1)
            crosshair_items.append(tick)

    draw_arc_ticks(cx, cy, arc_radius, 0, 180, tick_spacing_deg)
    draw_arc_ticks(cx, cy, arc_radius, 180, 180, tick_spacing_deg)












def draw_compass_slider(heading):
    for item in compass_labels:
        canvas.delete(item)
    compass_labels.clear()

    spacing = 12
    tick_interval = 5
    label_interval = 15  # finer ticks for smoother compass
    y_pos = 20
    base_line_y = y_pos + 10
    tick_height = 8

    edge_padding = 10
    left_limit = edge_padding
    right_limit = canvas_width - edge_padding

    # Compass baseline
    base_line = canvas.create_line(left_limit, base_line_y, right_limit, base_line_y, fill="lime", width=1)
    compass_labels.append(base_line)

    # End caps
    left_cap = canvas.create_line(left_limit, base_line_y - 5, left_limit, base_line_y + 15, fill="lime", width=1)
    right_cap = canvas.create_line(right_limit, base_line_y - 5, right_limit, base_line_y + 15, fill="lime", width=1)
    compass_labels.extend([left_cap, right_cap])

    # Degree and heading alignment
    heading_step = int(heading // tick_interval)
    offset_within_step = (heading % tick_interval) / tick_interval
    pixel_offset = offset_within_step * spacing
    center_deg = heading_step * tick_interval

    # Compass directions every 45°
    cardinal_map = {
        0: "N", 45: "NE", 90: "E", 135: "SE",
        180: "S", 225: "SW", 270: "W", 315: "NW"
    }

    for i in range(-18, 19):
        deg = int((center_deg + i * tick_interval) % 360)
        x = cx + i * spacing - pixel_offset

        if left_limit < x < right_limit:
            if deg % label_interval == 0:
                # === Major Tick ===
                tick = canvas.create_line(x, base_line_y - (tick_height / 2), x, base_line_y + tick_height, fill="lime", width=1)
                compass_labels.append(tick)

                # === Labels ===
                if deg in cardinal_map:
                    label_text = cardinal_map[deg]
                    label = canvas.create_text(x, y_pos - 8, text=label_text, fill="lime", font=("Courier", 10, "bold"))
                    sublabel = canvas.create_text(x, base_line_y + 15, text=f"{deg}°", fill="lime", font=("Courier", 7))
                    compass_labels.extend([label, sublabel])
                else:
                    sublabel = canvas.create_text(x, base_line_y + 15, text=f"{deg}°", fill="lime", font=("Courier", 7))
                    compass_labels.append(sublabel)
            else:
                # === Minor Tick (unlabeled) ===
                minor_tick = canvas.create_line(x, base_line_y, x, base_line_y + 4, fill="lime", width=1)
                compass_labels.append(minor_tick)


    # === Compass pointer (bold bracket + triangle) ===
    pointer_y = base_line_y + 18


    # Triangle arrow (pointing up)
    triangle = canvas.create_polygon(
        cx - 6, pointer_y + 10,
        cx + 6, pointer_y + 10,
        cx, pointer_y,
        fill="lime", outline="black", width=1
    )

    compass_labels.extend([triangle])



def draw_speed_dial(speed):
    for item in speed_dial_items:
        canvas.delete(item)
    speed_dial_items.clear()

    dial_radius = 15
    center_x = 30
    center_y = canvas_height - 23

    circle = canvas.create_oval(center_x - dial_radius, center_y - dial_radius,
                                center_x + dial_radius, center_y + dial_radius,
                                outline="lime", width=1)
    speed_dial_items.append(circle)

    angle = (speed % 100) * 2.7 - 135  # Map speed to angle
    angle_rad = math.radians(angle)
    needle_length = dial_radius - 5
    x_end = center_x + needle_length * math.cos(angle_rad)
    y_end = center_y + needle_length * math.sin(angle_rad)


    tick_count = 5
    for i in range(tick_count):
        tick_angle = -135 + i * (270 / (tick_count - 1))
        rad = math.radians(tick_angle)
        inner_r = dial_radius - 3
        outer_r = dial_radius
        x0 = center_x + inner_r * math.cos(rad)
        y0 = center_y + inner_r * math.sin(rad)
        x1 = center_x + outer_r * math.cos(rad)
        y1 = center_y + outer_r * math.sin(rad)
        tick = canvas.create_line(x0, y0, x1, y1, fill="lime", width=1)
        speed_dial_items.append(tick)

    needle = canvas.create_line(center_x, center_y, x_end, y_end, fill="lime", width=2)
    speed_dial_items.append(needle)

    label = canvas.create_text(center_x, center_y + dial_radius + 10, text="km/h", fill="lime", font=("Courier", 7))
    speed_dial_items.append(label)

def update_hud():
    global pitch_ladders

    t = time.time()
    heading = (t * 20) % 360
    pitch = 20 * math.sin(t / 1.5)
    roll = 30 * math.cos(t / 2.2)
    temperature = 25 + 2 * math.sin(t / 3)
    g_force = 1.0 + 0.2 * math.sin(t * 2)
    speed = 40 + 20 * math.sin(t / 1.2)
    altitude = 100 + 100 * math.sin(t / 4)  # Range from 0 to 200 m

    canvas.itemconfig(heading_readout_text, text=f"{heading:.1f}°")

    if temperature > 25:
        canvas.itemconfig(temp_text, text=f"⚠️{temperature:.1f}°C", fill="red")
    else:
        canvas.itemconfig(temp_text, text=f"{temperature:.1f}°C", fill="lime")

    if g_force > 1.0:
        canvas.itemconfig(gforce_text, text=f"⚠️{g_force:.2f}G", fill="red")
    else:
        canvas.itemconfig(gforce_text, text=f"{g_force:.2f}G", fill="lime")

    canvas.itemconfig(speed_text, text=f"{speed:.0f}km/h")

    for item in temp_bar_items:
        canvas.delete(item)
    temp_bar_items.clear()

    for item in gforce_bar_items:
        canvas.delete(item)
    gforce_bar_items.clear()

    bar_height = 100
    bar_top = cy - bar_height // 2
    bar_x = 40
    temp_ratio = min(max((temperature - 20) / 10, 0), 1)
    bar_fill_height = int(bar_height * temp_ratio)
    temp_color = "red" if temperature > 25 else "lime"

    temp_bar_outline = canvas.create_rectangle(bar_x, bar_top, bar_x + 5, bar_top + bar_height, outline="gray", width=1)
    temp_bar_fill = canvas.create_rectangle(bar_x, bar_top + bar_height - bar_fill_height, bar_x + 5, bar_top + bar_height, fill=temp_color, outline="")
    temp_bar_items.extend([temp_bar_outline, temp_bar_fill])

    for i in range(6):
        value = 20 + i * 2
        y = bar_top + bar_height - int((i / 5) * bar_height)
        label = canvas.create_text(bar_x - 10, y, text=f"{value}°", fill="gray", font=("Courier", 7), anchor="e")
        tick = canvas.create_line(bar_x - 3, y, bar_x, y, fill="gray")
        temp_bar_items.extend([label, tick])

    bar_x_right = canvas_width - 40
    g_ratio = min(max((g_force - 0.8) / 0.8, 0), 1)
    g_fill_height = int(bar_height * g_ratio)
    g_color = "red" if g_force > 1.0 else "lime"

    g_bar_outline = canvas.create_rectangle(bar_x_right, bar_top, bar_x_right + 5, bar_top + bar_height, outline="gray", width=1)
    g_bar_fill = canvas.create_rectangle(bar_x_right, bar_top + bar_height - g_fill_height, bar_x_right + 5, bar_top + bar_height, fill=g_color, outline="")
    gforce_bar_items.extend([g_bar_outline, g_bar_fill])

    for i in range(9):
        value = 0.8 + i * 0.1
        y = bar_top + bar_height - int((i / 8) * bar_height)
        label = canvas.create_text(bar_x_right + 15, y, text=f"{value:.1f}", fill="gray", font=("Courier", 7), anchor="w")
        tick = canvas.create_line(bar_x_right + 5, y, bar_x_right + 8, y, fill="gray")
        gforce_bar_items.extend([label, tick])

    draw_crosshair(roll)
    draw_compass_slider(heading)
    draw_speed_dial(speed)
    draw_altitude_dial(altitude)

    for item in pitch_ladders:
        canvas.delete(item)
    pitch_ladders.clear()

    safe_top = 65
    safe_bottom = canvas_height - 40
    rung_width = 40
    for angle in range(-40, 45, 10):
        y_offset = (angle - pitch) * -2
        if safe_top < cy + y_offset < safe_bottom:
            x0, y0 = -rung_width / 2, y_offset
            x1, y1 = rung_width / 2, y_offset
            roll_rad = math.radians(roll)
            x0r, y0r = rotate_point(x0, y0, roll_rad)
            x1r, y1r = rotate_point(x1, y1, roll_rad)
            line = canvas.create_line(cx + x0r, cy + y0r, cx + x1r, cy + y1r, fill="gray", width=1)
            pitch_ladders.append(line)
            label_x = cx + x1r + 5
            label_y = cy + y1r - 5
            label = canvas.create_text(label_x, label_y, text=f"{-angle}", fill="gray", font=("Courier", 8), anchor="w")
            pitch_ladders.append(label)

    root.after(33, update_hud)

# Initialize
draw_crosshair(0)
draw_compass_slider(0)
draw_speed_dial(0)
draw_altitude_dial(0)
update_hud()
root.mainloop()
